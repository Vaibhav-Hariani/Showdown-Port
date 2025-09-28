import numpy as np 
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
                if poke_id == 0:
                    continue  # Skip missingNo
                if is_active:
                    active_index = len(team)

                moves = [ShowdownParser.unpack_move(int(row[1 + k])) for k in range(4)]
                hp = int(row[5])
                status = ShowdownParser.unpack_status(int(row[6]))
                if is_active:
                    stat_mods = ShowdownParser.unpack_stat_mods(int(row[7]), int(row[8]))
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
