import csv

if __name__ == "__main__":
    with open('data/movedex.csv', 'r') as f:
        reader = csv.DictReader(f)
        moves = [row for row in reader]
    
    rendered_move_names = [
        move['name'].replace(' ', '_').replace('-', '_').upper() + "_MOVE_ID"
        for move in moves
    ]

    with open('sim/generated_move_enum.h', 'w') as f:
        f.write("// This file is auto-generated from data/write_move_array.py\n")
        f.write("#ifndef GENERATED_MOVE_ENUM_H\n")
        f.write("#define GENERATED_MOVE_ENUM_H\n\n")
        f.write("typedef enum {\n")
        f.write("  NO_MOVE = 0,\n")
        for i, (move, rendered_move_name) in enumerate(zip(moves, rendered_move_names)):
            f.write(f"  {rendered_move_name} = {move['move_id']},\n")
        f.write("} MOVE_IDS;\n")

        f.write("\n#endif // GENERATED_MOVE_ENUM_H\n")

    with open('sim/generated_movedex.h', 'w') as f:
        f.write("// This file is auto-generated from data/write_move_array.py\n")
        f.write("#ifndef GENERATED_MOVEDEX_H\n")
        f.write("#define GENERATED_MOVEDEX_H\n\n")
        f.write("#include \"effectiveness.h\"\n")
        f.write("#include \"generated_move_enum.h\"\n")
        f.write("#include \"movedex.h\"\n")
        f.write("#include \"pokemon.h\"\n\n")

        f.write("const Move MOVES[] = {\n")
        # insert the no move entry
        f.write("    {.move_id = NO_MOVE,\n")
        f.write("     .type = NONE_TYPE,\n")
        f.write("     .category = STATUS_MOVE_CATEGORY,\n")
        f.write("     .pp = 0,\n")
        f.write("     .power = 0,\n")
        f.write("     .accuracy = -1,\n")
        f.write("     .priority = 0,\n")
        f.write("     .effect = NULL},\n")
        for move, rendered_move_name in zip(moves, rendered_move_names):
            f.write(f"    {{.move_id = {rendered_move_name},\n")
            f.write(f"     .type = {move['type'].upper()},\n")
            f.write(f"     .category = {move['category'].upper()}_MOVE_CATEGORY,\n")
            f.write(f"     .pp = {move['pp']},\n")
            f.write(f"     .power = {-1 if move['power'] == 'INF' else move['power']},\n")
            f.write(f"     .accuracy = {-1 if move['accuracy'] == 'INF' else float(move['accuracy'])/100.0},\n")
            f.write(f"     .priority = {move['priority']},\n")
            f.write(f"     .effect = {move['effect_fn']}}},\n")
        f.write("};\n\n")

        f.write("\n#endif // GENERATED_MOVEDEX_H\n")

