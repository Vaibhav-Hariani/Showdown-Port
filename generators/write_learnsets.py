import ast
import csv


def render_move_name(move):
    return move.replace(" ", "_").replace("-", "_").upper() + "_MOVE_ID"


if __name__ == "__main__":
    with open("raw_data/learnsets.csv", "r") as f:
        reader = csv.DictReader(f)
        # Create a list of learnsets for each Pokemon
        rows = [
            {
                "pokedex_number": int(row["pokedex_number"]),
                "attacks": [
                    render_move_name(move)
                    for move in dict.fromkeys(ast.literal_eval(row["attacks"])).keys()
                ],
            }
            for row in reader
        ]
        # Sort by pokedex number to ensure consistent ordering
        rows.sort(key=lambda x: x["pokedex_number"])
        learnsets = [row["attacks"] for row in rows]

    # find and pad to the longest learnset size
    max_len = max(len(learnset) for learnset in learnsets)
    original_lens = [len(learnset) for learnset in learnsets]
    for i, learnset in enumerate(learnsets):
        if len(learnset) < max_len:
            learnsets[i] = learnset + ["NO_MOVE"] * (max_len - len(learnset))

    with open("data_sim/generated_learnsets.h", "w") as f:
        f.write("// This file is auto-generated from data/write_learnsets.py\n")
        f.write("#ifndef GENERATED_LEARNSETS_H\n")
        f.write("#define GENERATED_LEARNSETS_H\n\n")

        f.write('#include "generated_move_enum.h"\n\n')

        # write out the learnsets
        f.write(f"const MOVE_IDS LEARNSETS[{1 + len(original_lens)}][{max_len}] = {{\n")
        # Insert a "Missingno" entry at index 0
        missingo_learnset = ["NO_MOVE"] * max_len
        f.write(f" {{ {', '.join(missingo_learnset)} }},\n")
        for learnset in learnsets:
            # The csv contains a string representation of a list of moves
            # e.g. "['TACKLE', 'GROWL']"
            # TODO: Maybe switch to json lines which has a native list format?
            f.write(f" {{ {', '.join(learnset)} }},\n")
        f.write("};\n")

        # Write in the lengths of each learnset
        f.write(f"\nconst int LEARNSET_LENGTHS[{1 + len(original_lens)}] = {{")
        # Insert a 0 length for Missingno
        f.write("0, ")
        f.write(", ".join(str(l) for l in original_lens))
        f.write("};\n")
        f.write("\n#endif // GENERATED_LEARNSETS_H\n")
