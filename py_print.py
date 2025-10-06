import numpy as np 
from .data_labels.pokedex_labels import pokemon_name
from .data_labels.move_labels import move_name
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
        obs = np.asarray(flat_obs)
        if obs.ndim != 1:
            raise ValueError(f"Expected 1D observation, got shape {obs.shape}")
        expected = (1 + 2 * team_size) * 9
        if obs.size != expected:
            raise ValueError(
                f"Unexpected observation size {obs.size}; expected {expected}"
            )

        offset = 9
        players = []
        rows = obs[offset:].reshape(2 * team_size, 9)
        # Interleaved ordering: (p1_0, p2_0, p1_1, p2_1, ...)
        for p_idx in range(2):
            team = []
            active_index = None
            for j in range(team_size):
                row_index = j * 2 + p_idx
                row = rows[row_index]
                raw_id = int(row[0])
                is_active = raw_id < 0
                poke_id = abs(raw_id)
                if poke_id == 0:
                    continue  # Skip missingNo
                if is_active:
                    active_index = len(team)

                moves = [ShowdownParser.unpack_move(int(row[1 + k])) for k in range(4)]
                hp = int(row[5])
                status = ShowdownParser.unpack_status(int(row[6]))
                stat_mods = ShowdownParser.unpack_stat_mods(int(row[7]), int(row[8])) if is_active else None

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
            players.append({"active_index": active_index, "team": team})
        return {"players": players}

    @staticmethod
    def parse_observation_fast(obs: np.ndarray, team_size: int = 6) -> dict:
        obs = np.asarray(obs)
        offset = 9
        rows = obs[offset:].reshape(2 * team_size, 9)

        status_names = ["paralyzed", "burn", "freeze", "poison", "sleep", "confused"]

        p1_row = rows[0]
        p2_row = rows[1]

        def fmt(row):
            status_flags = int(row[6]) & 0xFFFF
            status_list = [status_names[i] for i in range(6) if (status_flags >> i) & 0x1]
            status_str = ','.join(status_list) if status_list else "healthy"
            move_id_parts = []
            move_name_parts = []
            for k in range(4):
                move_packed = int(row[1 + k]) & 0xFFFF
                move_id = move_packed & 0xFF
                if move_id > 0:
                    pp = (move_packed >> 8) & 0x3F
                    move_id_parts.append(f"{move_id}({pp})")
                    move_name_parts.append(f"{move_name(move_id)}({pp})")
            poke_id = abs(int(row[0]))
            return {
                'id': poke_id,
                'name': pokemon_name(poke_id),
                'hp': int(row[5]),
                'status': status_str,
                'moves': ','.join(move_id_parts),
                'move_names': ','.join(move_name_parts)
            }

        # Previous actions (both players)
        p1_type = int(obs[0])
        p1_val = int(obs[1])
        p2_type = int(obs[2])
        p2_val = int(obs[3])

        p1_move_name = move_name(p1_val) if p1_type == 2 else None
        p2_move_name = move_name(p2_val) if p2_type == 2 else None

        out = {
            'p1_active': fmt(p1_row),
            'p2_active': fmt(p2_row),
            # Backward compatibility
            'prev_action_type': p1_type,
            'prev_action_value': p1_val,
            # Explicit both-player fields
            'prev_p1_type': p1_type,
            'prev_p1_value': p1_val,
            'prev_p1_move_name': p1_move_name,
            'prev_p2_type': p2_type,
            'prev_p2_value': p2_val,
            'prev_p2_move_name': p2_move_name,
        }
        return out
    
    @staticmethod
    def format_status_fast(status: dict, status_names: list) -> str:
        """Fast status formatting using pre-computed names"""
        # Use list comp with direct indexing instead of dict iteration
        active = [name for name in status_names if status.get(name, False)]
        return ','.join(active) if active else "healthy"

    @staticmethod
    def pretty_print(obs):
        data_dict = ShowdownParser.parse_observation(obs)
        players = data_dict["players"]
        for idx, player in enumerate(players, start=1):
            print(f"Player {idx}:")
            for i, mon in enumerate(player["team"]):
                active_marker = "*" if i == player["active_index"] else " "
                status_flags = [name for name, val in mon["status"].items() if val]
                status_str = ", ".join(status_flags) if status_flags else "healthy"
                moves_str = ", ".join([f"{m['id']}({m['pp']})" for m in mon["moves"]])
                print(f"  {active_marker}Pokemon {mon['id']} | HP {mon['hp']} | Status: {status_str} | Moves: {moves_str}")
