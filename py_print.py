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
        """Parse v3 (88-int) packed observation only. Returns normalized dict."""
        obs = np.asarray(flat_obs)
        if obs.ndim != 1:
            raise ValueError(f"Expected 1D observation, got shape {obs.shape}")
        if obs.size != 88:
            raise ValueError(
                f"Expected v3 packed observation of length 88, got {obs.size}"
            )
        return ShowdownParser._parse_v3(obs, team_size)

    @staticmethod
    def _parse_v3(obs: np.ndarray, team_size: int):
        # Layout: header (4) + rows (interleaved) * 7
        # Header contains active stat modifiers for p1 (words 0,1) and p2 (words 2,3)
        if obs.size != (4 + 2 * team_size * 7):
            raise ValueError(f"Unexpected obs length for v3: {obs.size}")

        header = obs[:4].astype(np.int32)
        p1_statmods = ShowdownParser.unpack_stat_mods(int(header[0]), int(header[1]))
        p2_statmods = ShowdownParser.unpack_stat_mods(int(header[2]), int(header[3]))

        rows = obs[4:].reshape(2 * team_size, 7).astype(np.int32)  # interleaved rows
        players = []

        for p_idx in range(2):
            active_index = None
            active_pokemon = None
            for slot in range(team_size):
                row = rows[slot * 2 + p_idx]
                species = int(row[0])
                # Active Pokémon are stored as negated species id
                if species >= 0:
                    # not the active pokemon from observer perspective
                    continue

                active_index = slot
                species_id = -species

                # Moves are packed in row[1..4]
                moves = []
                for k in range(4):
                    packed = int(row[1 + k])
                    if packed == 0:
                        # move not revealed / not present
                        continue
                    mv = ShowdownParser.unpack_move(packed)
                    mv["name"] = (
                        move_name(int(mv["id"])) if mv["id"] is not None else None
                    )
                    moves.append(mv)

                hp_scaled = int(row[5])
                status_flags = int(row[6])

                # stat_mods for the active Pokemon come from the header
                stat_mods = p1_statmods if p_idx == 0 else p2_statmods

                active_pokemon = {
                    "id": species_id,
                    "name": pokemon_name(species_id),
                    "hp_scaled": hp_scaled,
                    "status": ShowdownParser.unpack_status(status_flags),
                    "stat_mods": stat_mods,
                    "moves": moves,
                }
                # only one active per team; break after found
                break

            players.append(
                {
                    "active_index": active_index,
                    "active_pokemon": active_pokemon,
                }
            )

        return {
            "header": {"p1_statmods": p1_statmods, "p2_statmods": p2_statmods},
            "players": players,
        }
