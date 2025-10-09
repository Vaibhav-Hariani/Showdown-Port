import numpy as np 
from pokedex_labels import pokemon_name
from move_labels import move_name

class ShowdownParser:
    @staticmethod
    def unpack_status(flags: int) -> dict:
        f = int(flags) & 0xFFFF
        return {
            "paralyzed": bool((f >> 0) & 0x1),
            "burn": bool((f >> 1) & 0x1),
            "freeze": bool((f >> 2) & 0x1),
            "poison": bool((f >> 3) & 0x1),
            "sleep": bool((f >> 4) & 0x1),
            "confused": bool((f >> 5) & 0x1),
        }

    @staticmethod
    def unpack_stat_mods(word1: int, word2: int) -> dict:
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

    @staticmethod
    def unpack_move(packed: int) -> dict:
        x = int(packed) & 0xFFFF
        move_id = x & 0xFF
        pp = (x >> 8) & 0x3F
        return {"id": move_id, "pp": pp}

    @staticmethod
    def parse_observation(flat_obs: np.ndarray, team_size: int = 6) -> dict:
        """Parse v2 (92-int) packed observation only. Returns normalized dict."""
        obs = np.asarray(flat_obs)
        if obs.ndim != 1:
            raise ValueError(f"Expected 1D observation, got shape {obs.shape}")
        if obs.size != 92:
            raise ValueError(f"Expected v2 packed observation of length 92, got {obs.size}")
        return ShowdownParser._parse_v2(obs, team_size)



    @staticmethod
    def _parse_v2(obs: np.ndarray, team_size: int):
        # Layout: header (8) + rows (interleaved) * 7
        players = []
        rows = obs[8:].reshape(2 * team_size, 7)
        for p_idx in range(2):
            team = []
            active_index = None
            for j in range(team_size):
                row = rows[j * 2 + p_idx]
                raw_id = int(row[0])
                is_active = raw_id < 0 and raw_id != 0
                poke_id = abs(raw_id)
                if poke_id == 0:
                    # Hidden / unrevealed opponent slot
                    continue
                if is_active:
                    active_index = len(team)
                moves = []
                for k in range(4):
                    m = ShowdownParser.unpack_move(int(row[1 + k]))
                    m['name'] = move_name(m['id']) if m['id'] else None
                    moves.append(m)
                hp_scaled = row[5] / 10  # 0..1000
                status = ShowdownParser.unpack_status(int(row[6]))
                team.append({
                    'id': poke_id,
                    'name': pokemon_name(poke_id),
                    'is_active': is_active,
                    'hp_scaled': hp_scaled,
                    'status': status,
                    'moves': moves,
                })
            players.append({'active_index': active_index, 'team': team})
        # Active stat mods live only in header for v2
        header = {
            'p1_prev_choice': int(obs[0]),
            'p1_prev_val': int(obs[1]),
            'p2_prev_choice': int(obs[2]),
            'p2_prev_val': int(obs[3]),
            'p1_statmods': ShowdownParser.unpack_stat_mods(int(obs[4]), int(obs[5])),
            'p2_statmods': ShowdownParser.unpack_stat_mods(int(obs[6]), int(obs[7])),
        }
        return {'version': 'v2', 'players': players, 'header': header}


    
    @staticmethod
    def format_status_fast(status: dict, status_names: list) -> str:
        """Fast status formatting using pre-computed names"""
        # Use list comp with direct indexing instead of dict iteration
        active = [name for name in status_names if status.get(name, False)]
        return ','.join(active) if active else "healthy"

    @staticmethod
    def pretty_print(obs):
        data_dict = ShowdownParser.parse_observation(obs)
        header = data_dict['header']
        players = data_dict['players']
        print(f"Header: {header}")
        for idx, player in enumerate(players, start=1):
            print(f"Player {idx}:")
            for i, mon in enumerate(player['team']):
                active_marker = '*' if i == player['active_index'] else ' '
                status_flags = [name for name, val in mon['status'].items() if val]
                status_str = ','.join(status_flags) if status_flags else 'healthy'
                hp_field = mon.get('hp', mon.get('hp_scaled'))
                moves_str = ', '.join([f"{m['id']}({m['pp']})" for m in mon['moves'] if m['id']])
                print(f"  {active_marker}Pokemon {mon['id']} | HP {hp_field} | Status: {status_str} | Moves: {moves_str}")

    # --- Comparison Utilities ---
    @staticmethod
    def compare_v2(packed_a: np.ndarray, packed_b: np.ndarray, team_size: int = 6):
        """Compare two v2 packed observations field-by-field; return dict of mismatches."""
        a = ShowdownParser.parse_observation(packed_a, team_size)
        b = ShowdownParser.parse_observation(packed_b, team_size)
        mismatches = {}
        # Compare header stat mods
        for k in ['p1_statmods', 'p2_statmods']:
            for stat, aval in a['header'][k].items():
                bval = b['header'][k][stat]
                if aval != bval:
                    mismatches.setdefault(k, {})[stat] = (aval, bval)
        # Compare active pokemon basic info
        for pi in range(2):
            ateam = a['players'][pi]['team']
            bteam = b['players'][pi]['team']
            limit = min(len(ateam), len(bteam))
            for i in range(limit):
                af = ateam[i]; bf = bteam[i]
                # Only check visible species and active status
                for field in ['id', 'is_active']:
                    if af[field] != bf[field]:
                        mismatches.setdefault(f'player{pi+1}_mon{i}', {})[field] = (af[field], bf[field])
                # HP scaled
                ahp = af.get('hp_scaled', af.get('hp'))
                bhp = bf.get('hp_scaled', bf.get('hp'))
                if ahp != bhp:
                    mismatches.setdefault(f'player{pi+1}_mon{i}', {})['hp'] = (ahp, bhp)
                # Moves (only revealed ones: id>0)
                for mi in range(4):
                    amid = af['moves'][mi]['id']
                    bmid = bf['moves'][mi]['id']
                    if amid != bmid:
                        # Ignore if one side is unrevealed (0) and the other also 0
                        if not (amid == 0 and bmid == 0):
                            mismatches.setdefault(f'player{pi+1}_mon{i}', {})[f'move{mi}'] = (amid, bmid)
        return mismatches

    @staticmethod
    def assert_equivalent(packed_a: np.ndarray, packed_b: np.ndarray):
        mismatches = ShowdownParser.compare_v2(packed_a, packed_b)
        if mismatches:
            print("Mismatch detected:")
            for k, v in mismatches.items():
                print(f"  {k}: {v}")
        else:
            print("Observations equivalent (v2 criteria).")
