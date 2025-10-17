import numpy as np
import asyncio
import time
import torch
from gymnasium.spaces import Space, Box

from gymnasium.utils.env_checker import check_env
from poke_env.battle import AbstractBattle
from poke_env.battle import Pokemon, Move, AbstractBattle
from poke_env.player import Player, RandomPlayer, SimpleHeuristicsPlayer
from poke_env.teambuilder import Teambuilder
from poke_env.environment import SinglesEnv

from move_labels import MOVE_LABELS
from pokedex_labels import POKEMON_NAMES
from ou_teams import get_random_ou_team, OU_TEAMS

from pufferlib.ocean.showdown_models import Showdown, ShowdownLSTM

class Embed():
    def embed_battle(self, battle: AbstractBattle):
        return self._packed_embed(battle)
     
    # Helpers for packed form --------------------------------------------
    @staticmethod
    def _encode_hp_percent(mon):
        if not mon or mon.max_hp == 0:
            return 0
        frac = mon.current_hp_fraction
        if frac < 0:
            frac = 0
        if frac > 1:
            frac = 1
        return int(frac * 1000) & 0x3FF  # 10 bits

    @staticmethod
    def _encode_boost(val):
        # boost in [-6, +6]; store (val + 6) clamped 0..12 (fits 4 bits with clip)
        if val is None:
            val = 0
        v = val + 6
        if v < 0:
            v = 0
        if v > 12:
            v = 12
        return v & 0xF

    @staticmethod
    def _pack_move(move, revealed=True, disabled=False):
        """(Legacy helper) C layout uses:
        bits0-7 move id, bits8-12 PP (0-31), bit13 disabled, bits14-15 reserved 0.
        We do NOT set any 'revealed' flag bit (parity with C). Hidden moves => 0.
        """
        if move is None or not revealed:
            return 0
        move_id = (move.id or 0) & 0xFF
        pp = int(getattr(move, "current_pp", getattr(move, "pp", 0)))
        if pp < 0: pp = 0
        if pp > 31: pp = 31
        val = move_id | (pp << 8)
        if disabled:
            val |= (1 << 13)
        # bits14-15 remain 0
        return val & 0x3FFF  # mask to 14 bits (bit13 included)

    def _pack_statmods1(self, p: Pokemon):
        # (Atk, Def, SpA, SpD) -> each 4 bits
        if not p:
            return 0
        boosts = p.boosts
        atk = self._encode_boost(boosts.get("atk", 0))
        de = self._encode_boost(boosts.get("def", 0))
        spa = self._encode_boost(boosts.get("spa", 0))
        spd = self._encode_boost(boosts.get("spd", 0))
        val = atk | (de << 4) | (spa << 8) | (spd << 12)
        # Ensure we return a Python int within signed int16 range while
        # preserving the raw 16-bit pattern (two's complement). This
        # avoids OverflowError when numpy attempts to cast large unsigned
        # values into np.int16. Consumers can reinterpret the bits as
        # unsigned if needed (e.g., via np.uint16).
        val &= 0xFFFF
        if val & 0x8000:
            val -= 0x10000
        return val

    def _pack_statmods2(self, p):
        if not p:
            return 0
        boosts = p.boosts
        spe = self._encode_boost(boosts.get("spe", 0))
        acc = self._encode_boost(boosts.get("accuracy", 0))
        eva = self._encode_boost(boosts.get("evasion", 0))
        val = spe | (acc << 4) | (eva << 8)
        val &= 0xFFFF
        if val & 0x8000:
            val -= 0x10000
        return val

    def _pack_status_bits(self, p):
        if not p:
            return 0
        bits = 0
        # Core status
        status = p.status.name if p.status else None
        if status == "PAR":
            bits |= 1 << 0
        elif status == "BRN":
            bits |= 1 << 1
        elif status == "FRZ":
            bits |= 1 << 2
        elif status in ("PSN", "PSN2"):
            bits |= 1 << 3
            if status == "PSN2":
                bits |= 1 << 5  # TOX
        elif status == "SLP":
            bits |= 1 << 4
        # Confusion tracking not always exposed in poke-env; ignore -> 0
        # Other Gen1 volatiles placeholders left 0
        if getattr(p, "fainted", False):
            pass  # no explicit bit; faint encoded via HP=0
        return bits

    def _packed_embed(self, battle: AbstractBattle):
        # New compact layout: total length 88 (mirrors C packing with previous-choice fields removed)
        # Deviations vs C implementation:
        #   - Previous choices (indices 0..3) unavailable in poke-env -> set to -1.
        #   - Opponent confusion / flinch bits not populated (poke-env lacks direct counters).
        #   - Disabled flag logic not implemented (always 0 as in current C path).
        #   - Hidden opponent species gating: opponent team dict only contains revealed mons.
        #   - Move revelation: we infer via mv.was_used; C sets Move.revealed also from prev choices.
        obs = np.zeros(88, dtype=np.int16)
        p1a = battle.active_pokemon
        p2a = battle.opponent_active_pokemon
        # statmods for p1 and p2 stored at indices 0..3
        obs[0] = self._pack_statmods1(p1a)
        obs[1] = self._pack_statmods2(p1a)
        obs[2] = self._pack_statmods1(p2a)
        obs[3] = self._pack_statmods2(p2a)

        def species_id(mon: Pokemon, active):
            # Use pokemon name to look up ID in POKEDEX_LABELS
            poke_name = mon.name.lower()
            # Handle special cases that might differ between poke-env and our labels
            name_mapping = {
                'nidoran-f': 'nidoranf',
                'nidoran-m': 'nidoranm',
                'mr. mime': 'mrmime',
                "farfetch’d": 'farfetchd'
            }
            poke_name = name_mapping.get(poke_name, poke_name)
            sid = POKEMON_NAMES.index(poke_name)
            return -sid if active else sid

        # NOTE: C sim uses fixed original team order. poke-env may not expose
        # unrevealed opponents yet; we approximate with a stable per-battle slot map.
        def ordered_team(team):
            # Preserve insertion order (dict preserves insertion order in Python 3.7+)
            mons = list(team.values())[:6]
            return mons + [None]*(6-len(mons))

        p1_team = ordered_team(battle.team)
        
        # Use natural ordering from poke-env - no caching needed
        p2_team = ordered_team(battle.opponent_team)

        def pack_move_py(move: Move, disabled=False):
            # Use move name to look up ID in MOVE_LABELS
            if move is None:
                return 0
            move_name = move.id.lower()
            move_id = MOVE_LABELS.index(move_name) & 0xFF
            pp = int(getattr(move, 'current_pp', getattr(move, 'pp', 0)))
            pp = pp & 0x3F  # 6 bits for PP (0-63)
            val = move_id | (pp << 8)
            if disabled:
                val |= (1 << 14)  # bit 14 for disabled flag
            return val

        for slot in range(6):
            base = 4 + (slot*2)*7
            mon = p1_team[slot]
            if mon:
                obs[base+0] = species_id(mon, mon is p1a)
                moves = list(mon.moves.values()) if hasattr(mon,'moves') else []
                for mi in range(4):
                    mv = moves[mi] if mi < len(moves) else None
                    obs[base+1+mi] = pack_move_py(mv)
                obs[base+5] = self._encode_hp_percent(mon)
                obs[base+6] = self._pack_status_bits(mon)
            base2 = 4 + (slot*2+1)*7
            mon2 = p2_team[slot]
            if mon2:
                # Species id always present once mon object exists (considered revealed)
                obs[base2+0] = species_id(mon2, mon2 is p2a)
                if hasattr(mon2,'moves'):
                    moves2 = list(mon2.moves.values())
                    for mi in range(4):
                        mv = moves2[mi] if mi < len(moves2) else None
                        # Show all opponent moves that are available
                        if mv:
                            obs[base2+1+mi] = pack_move_py(mv)
                obs[base2+5] = self._encode_hp_percent(mon2)
                obs[base2+6] = self._pack_status_bits(mon2)
        return obs

    # Modify this to return zero if the game isn't over, else 1 if the model won, -1 if the model lost.
    def calc_reward(self, battle: AbstractBattle) -> float:
        return self.reward_computing_helper(
            battle, fainted_value=2.0, hp_value=1.0, victory_value=30.0
        )
    

class Env(SinglesEnv):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        # Define observation space to match C sim: 88 int16 values (choices removed)
        # 4 header stat ints + 12*7 = 84 ints (rows) = 88 total
        obs_size = 88
        low = np.full(obs_size, -32768, dtype=np.int16)
        high = np.full(obs_size, 32767, dtype=np.int16)
        self.observation_spaces = {
            agent: Box(low, high, dtype=np.int16) for agent in self.possible_agents
        }


# class SmartAgent(Player):
#     """Simple agent that chooses the move with highest base power.
#     """
#     def __init__(self, **kwargs):
#         super().__init__(**kwargs)

#     def choose_move(self, battle: AbstractBattle):
#         # Choose the move with the highest base power
#         if battle.available_moves:
#             best_move_idx = 0
#             best_power = -1
            
#             for i, move in enumerate(battle.available_moves):
#                 # Get base power, default to 0 for status moves
#                 power = move.base_power if move.base_power else 0
                
#                 if power > best_power:
#                     best_power = power
#                     best_move_idx = i
            
#             # Use the best move
#             return self.create_order(battle.available_moves[best_move_idx])
#         else:
#             # No moves available, try to switch to first available switch
#             if battle.available_switches:
#                 return self.create_order(battle.available_switches[0])
        
#         # Ultimate fallback
#         return self.choose_random_move(battle)
    

class OUTeambuilder(Teambuilder):
    """Teambuilder that randomly selects from predefined RBY OU teams."""
    
    def __init__(self):
        super().__init__()
        # Convert all OU teams to showdown format and parse them
        self.packed_teams = []
        
        for team_data in OU_TEAMS:
            # Convert team_data format to showdown format string
            showdown_team = self._convert_to_showdown_format(team_data)
            # Parse and pack the team
            parsed_team = self.parse_showdown_team(showdown_team)
            packed_team = self.join_team(parsed_team)
            self.packed_teams.append(packed_team)
    
    def _convert_to_showdown_format(self, team_data):
        """Convert OU team format to showdown format string."""
        pokemon_strings = []
        for pokemon in team_data:
            species = pokemon['species']
            moves = pokemon['moves']
            # Basic showdown format for Gen 1
            # Species @ Item (gen1 has no items, so just species)
            # - Move1
            # - Move2
            # - Move3
            # - Move4
            pokemon_str = f"{species}\n"
            for move in moves:
                pokemon_str += f"- {move}\n"
            pokemon_strings.append(pokemon_str)
        
        return "\n".join(pokemon_strings)
    
    def yield_team(self):
        """Return a random packed team."""
        import random
        return random.choice(self.packed_teams)


class RLAgent(Player):
    """RL agent that loads a PyTorch model and uses its output vector for action selection.
    Uses the same action parsing logic as SmartAgent.
    Includes a teambuilder that selects random teams from the OU teams list.
    """
    def __init__(self, model_path: str, device='cuda', use_ou_teams=False, **kwargs):
        self.env = Env(battle_format=kwargs.get("battle_format", "gen1randombattle"))
        self.embed = Embed()        
        
        # If use_ou_teams is True and no team provided, create OUTeambuilder
        if use_ou_teams and 'team' not in kwargs:
            kwargs['team'] = OUTeambuilder()
        
        # Load the PyTorch model
        model = Showdown(None, hidden_size=512)
        self.model = ShowdownLSTM(None, policy=model, input_size=512, hidden_size=512)
        weights = torch.load(model_path, weights_only=False)

        self.model.load_state_dict(weights)
        self.model.to(device)  # Move model to device
        self.model.eval()  # Set to evaluation mode
        
        # Initialize eval state using the new method with specified device
        self.model.init_eval_state(device=device, batch_size=1)
        
        # Track which battle the state belongs to so we can reset on new battles
        self._last_battle_id = None
        super().__init__(**kwargs)

    # Parse model action (0-9) -> Showdown order (same as SmartAgent)
    def parse_action(self, battle: AbstractBattle, action):
        """
        Convert action vector to Pokemon Showdown order.
        
        Args:
            action: Probability vector of length 10 from model output
        """
     
        # Sort actions by probability (highest first)
        action_probs = [(i, prob) for i, prob in enumerate(action)]
        action_probs.sort(key=lambda x: x[1], reverse=True)
        
        # Try each action in order of preference
        for action_idx, prob in action_probs:
            if prob <= 0:
                continue
                
            # 0-5 switch slots
            if 0 <= action_idx <= 5:
                team_mons = list(battle.team.values())
                if action_idx < len(team_mons):
                    target_mon = team_mons[action_idx]
                    # Check if this Pokemon is in available_switches
                    if target_mon in battle.available_switches:
                        return self.create_order(target_mon) 
                                    
            # 6-9 move slots
            elif 6 <= action_idx <= 9:
                move_idx = action_idx - 6
                if move_idx < len(battle.available_moves):
                    return self.create_order(battle.available_moves[move_idx])
        
        # Fallback to random move
        return self.choose_random_move(battle)

    def choose_move(self, battle: AbstractBattle):
        # Get packed observation
        packed_obs = self.embed.embed_battle(battle)
        
        # Detect new battle and reset LSTM state when a new battle starts
        turn = battle.turn
        if turn == 1:
            self.model.reset_eval_state()
        
        # Use the new get_action method from ShowdownLSTM
        # It handles observation conversion, forward pass, and state updates
        action_idx, _ = self.model.get_action(packed_obs, deterministic=False)
        
        # Convert action index to action probabilities for parse_action
        # Create a one-hot-like distribution favoring the selected action
        action_probs = np.zeros(10, dtype=np.float32)
        action_probs[action_idx] = 1.0
        
        # Use parse_action with the probability vector
        return self.parse_action(battle, action_probs)


def benchmark_agents(n_battles=100):
    """Run multiple battles and collect statistics with timing data"""
    print(f"Starting benchmark with {n_battles} battles...")
    
    async def run_battles():
        # Create players
        smart_agent = SimpleHeuristicsPlayer(battle_format="gen1ou", team=OUTeambuilder())
        base_path = "/puffertank/Showdown_comp_env/bindings/comp_env_bindings/"
        model_path = "trained_balmy_moon.pt"
        rl_agent = RLAgent(model_path=f"{base_path}{model_path}", use_ou_teams=True, battle_format="gen1ou")

        # Track results and timing
        smart_wins = 0
        rl_wins = 0
        battle_times = []
        battle_moves = []
        # Start overall timing
        start_time = time.time()
        
        for i in range(n_battles):
            # Time each individual battle
            battle_start = time.time()
            
            # Play battle
            await rl_agent.battle_against(smart_agent, n_battles=1)
            
            battle_end = time.time()
            battle_duration = battle_end - battle_start
            battle_times.append(battle_duration)
            
            # Check results from the most recent battle
            if rl_agent.battles:
                battle_tag = list(rl_agent.battles.keys())[-1]  # Get the most recent battle
                battle = rl_agent.battles[battle_tag]

                # Track number of moves in this battle
                num_moves = battle.turn if hasattr(battle, 'turn') else 0
                battle_moves.append(num_moves)
                
                if battle.won:
                    rl_wins += 1
                else:
                    smart_wins += 1
                    
                if (i + 1) % 10 == 0:
                    avg_moves = np.mean(battle_moves) if battle_moves else 0
                    print(f"Completed {i + 1}/{n_battles} battles", end = '\r')
        
        # Calculate timing statistics
        total_time = time.time() - start_time
        avg_battle_time = np.mean(battle_times) if battle_times else 0
        min_battle_time = np.min(battle_times) if battle_times else 0
        max_battle_time = np.max(battle_times) if battle_times else 0
        
        # Calculate move statistics
        avg_moves = np.mean(battle_moves) if battle_moves else 0
        
        # Print results with timing data
        print(f"\n=== BENCHMARK RESULTS ===")
        print(f"Smart Agent wins: {smart_wins}/{n_battles} ({smart_wins/n_battles*100:.1f}%)")
        print(f"RL Agent wins: {rl_wins}/{n_battles} ({rl_wins/n_battles*100:.1f}%)")
        
        print(f"\n=== TIMING STATISTICS ===")
        print(f"Total time: {total_time:.2f} seconds ({total_time/60:.2f} minutes)")
        print(f"Fastest battle: {min_battle_time:.2f} seconds")
        print(f"Slowest battle: {max_battle_time:.2f} seconds")        
        print(f"Average time per move: {avg_battle_time/avg_moves*1000:.1f} ms")        
        return smart_wins, rl_wins
    
    # Run the async battles
    return asyncio.run(run_battles())


if __name__ == "__main__":
    print("Testing Pokemon agents with embed_battle pipeline...")
    
    # Test basic environment setup
    print("✓ Environment setup complete!")
    
    # # Collect some training data to show the pipeline
    # print("\nCollecting training data...")
    # training_data = collect_training_data(n_battles=5)
    
    # Run benchmark
    print("\nRunning agent benchmark...")
    benchmark_agents(n_battles=500)  # Run 100 battles to see timing data
