import gymnasium
import numpy as np

import pufferlib
from pufferlib.ocean.Showdown import binding


###############################
# Observation parsing helpers #
###############################


def _unpack_status(flags: int) -> dict:
    """Decode status flags packed in sim.h pack_status.

    Bits:
      0: paralyzed
      1: burn
      2: freeze
      3: poison
      4: sleep (boolean: any sleep>0)
      5: confused (set for active only)
    """
    f = int(flags) & 0xFFFF
    return {
        "paralyzed": bool((f >> 0) & 0x1),
        "burn": bool((f >> 1) & 0x1),
        "freeze": bool((f >> 2) & 0x1),
        "poison": bool((f >> 3) & 0x1),
        "sleep": bool((f >> 4) & 0x1),
        "confused": bool((f >> 5) & 0x1),
    }


def _unpack_stat_mods(word1: int, word2: int) -> dict:
    """Decode active Pokemon stat modifiers packed by:
    - pack_attack_def_specA_specD (word1)
    - pack_stat_acc_eva (word2)

    Each modifier is a 4-bit nibble (0..15). Mapping to in-game stages
    (-6..+6) is left to downstream logic if needed.
    """
    w1 = int(word1) & 0xFFFF
    w2 = int(word2) & 0xFFFF
    return {
        "attack": (w1 >> 0) & 0xF,
        "defense": (w1 >> 4) & 0xF,
        "special_attack": (w1 >> 8) & 0xF,
        "special_defense": (w1 >> 12) & 0xF,
        "speed": (w2 >> 0) & 0xF,
        "accuracy": (w2 >> 4) & 0xF,
        "evasion": (w2 >> 8) & 0xF,
    }


def _unpack_move(packed: int) -> dict:
    """Decode a move packed by sim.h pack_move.

    Layout (int16): bits 0-7 = move_id, bits 8-13 = pp.
    """
    x = int(packed) & 0xFFFF
    move_id = x & 0xFF
    pp = (x >> 8) & 0x3F
    return {"id": move_id, "pp": pp}


def parse_observation(flat_obs: np.ndarray, team_size: int = 6) -> dict:
    """Parse a single flattened observation vector (shape == (108,)) into a structured dict.

    This mirrors the layout produced by pack_battle in sim.h:
      For each of 2 players, for each of 6 Pokemon (row size = 9):
        [id, move1, move2, move3, move4, hp, status_flags, stat_mod1, stat_mod2]

    Notes:
      - Active Pokemon are marked by a negative id. We report absolute id and set is_active.
      - Non-active Pokemon have stat_mod1 and stat_mod2 set to 0.
      - status_flags bit 5 (confused) is only set for the active Pokemon.
    """
    obs = np.asarray(flat_obs)
    if obs.ndim != 1:
        raise ValueError(f"Expected 1D observation, got shape {obs.shape}")
    if obs.size != 2 * team_size * 9:
        raise ValueError(
            f"Unexpected observation size {obs.size}; expected {2 * team_size * 9}"
        )

    players = []
    rows = obs.reshape(2 * team_size, 9)
    for p_idx in range(2):
        team = []
        active_index = None
        for j in range(team_size):
            row = rows[p_idx * team_size + j]
            raw_id = int(row[0])
            is_active = raw_id < 0
            poke_id = abs(raw_id)
            if is_active:
                active_index = j

            # Move data is now at positions 1-4
            moves = [_unpack_move(int(row[1 + k])) for k in range(4)]

            # HP is now at position 5
            hp = int(row[5])

            # Status flags are now at position 6
            status = _unpack_status(int(row[6]))

            # Stat mods are at positions 7 and 8 (only meaningful for active mon; zeros otherwise)
            if is_active:
                stat_mods = _unpack_stat_mods(int(row[7]), int(row[8]))
            else:
                stat_mods = None

            team.append(
                {
                    "id": poke_id,
                    "is_active": is_active,
                    "hp": hp,
                    "status": status,
                    "stat_mods": stat_mods,
                    "moves": moves,
                }
            )

        players.append(
            {
                "active_index": active_index,
                "team": team,
            }
        )

    return {"players": players}


def min_obs(data_dict: dict):
    """Print active Pokémon, HP, and any active status flags for both players."""
    players = data_dict["players"]
    for idx, player in enumerate(players, start=1):
        active_idx = player["active_index"]
        team = player["team"]
        mon = team[active_idx]
        mon_id = mon["id"]
        hp = mon["hp"]
        status = mon["status"]
        active_flags = [name for name, val in status.items() if val]
        status_str = ", ".join(active_flags) if active_flags else "healthy"
        print(f"Player {idx}: Pokemon {mon_id} | HP {hp} | Status: {status_str}")


class Sim(pufferlib.PufferEnv):
    def __init__(
        self, num_envs=1, render_mode=None, log_interval=128, buf=None, seed=0
    ):
        ##Dims for observations = 6 (Pokemon) * 2(Players) * 9 (Entries per pokemon) = 108
        # Each pokemon is represented by: [id, move1, move2, move3, move4, hp, status_flags, stat_mod1, stat_mod2]
        self.single_observation_space = gymnasium.spaces.Box(
            low=-32768, high=32767, shape=(108,), dtype=np.int16
        )
        self.single_action_space = gymnasium.spaces.Discrete(10)
        self.render_mode = render_mode
        self.num_agents = num_envs
        self.log_interval = log_interval

        super().__init__(buf)
        self.c_envs = binding.vec_init(
            self.observations,
            self.actions,
            self.rewards,
            self.terminals,
            self.truncations,
            num_envs,
            seed,
        )

    def reset(self, seed=0, options=None):
        if options:
            ## Reset a specific environment
            binding.env_reset(self.c_envs + options, seed)
        binding.vec_reset(self.c_envs, seed)
        self.tick = 0
        return self.observations, []

    def step(self, actions):
        self.tick += 1
        self.actions[:] = actions
        binding.vec_step(self.c_envs)
        info = []
        if self.tick % self.log_interval == 0:
            info.append(binding.vec_log(self.c_envs))
        return (self.observations, self.rewards, self.terminals, self.truncations, info)

    def render(self):
        binding.vec_render(self.c_envs, 0)
        return parse_observation(self.observations)

    def close(self):
        binding.vec_close(self.c_envs)


if __name__ == "__main__":
    N = 8192
    env = Sim(num_envs=N)
    env.reset(seed=42)
    steps = 0
    CACHE = 1024
    actions = np.random.randint(0, 10, (CACHE, N))
    i = 0
    import time

    print("Starting now")
    start = time.time()
    while time.time() - start < 10:
        obs, rewards, terminals, trunc, info = env.step(actions[i % CACHE])
        # obs_dict = parse_observation(obs[0])
        # min_obs(obs_dict)
        steps += N
        i += 1
        # print(steps, 'steps in', time.time() - start, 'seconds', end='\r')
    print("\n Showdown SPS:", int(steps / (time.time() - start)))
