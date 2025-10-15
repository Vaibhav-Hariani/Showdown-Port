#!/usr/bin/env python3
"""
Generate RBY OU sample teams header file.
Since we cannot access the URLs, we'll use well-known RBY OU teams
from the competitive metagame.
"""

import os

# Common RBY OU teams based on the metagame
# Format: list of dicts with pokemon name and moves
SAMPLE_TEAMS = [
    # Standard balance team
    [
        {"name": "TAUROS", "moves": ["BODY_SLAM", "HYPER_BEAM", "EARTHQUAKE", "BLIZZARD"]},
        {"name": "CHANSEY", "moves": ["ICE_BEAM", "THUNDER_WAVE", "SOFT_BOILED", "SEISMIC_TOSS"]},
        {"name": "EXEGGUTOR", "moves": ["PSYCHIC", "SLEEP_POWDER", "STUN_SPORE", "EXPLOSION"]},
        {"name": "SNORLAX", "moves": ["BODY_SLAM", "EARTHQUAKE", "HYPER_BEAM", "REST"]},
        {"name": "ALAKAZAM", "moves": ["PSYCHIC", "SEISMIC_TOSS", "THUNDER_WAVE", "RECOVER"]},
        {"name": "STARMIE", "moves": ["SURF", "BLIZZARD", "THUNDER_WAVE", "RECOVER"]},
    ],
    # Offensive team
    [
        {"name": "TAUROS", "moves": ["BODY_SLAM", "HYPER_BEAM", "EARTHQUAKE", "BLIZZARD"]},
        {"name": "SNORLAX", "moves": ["BODY_SLAM", "EARTHQUAKE", "HYPER_BEAM", "AMNESIA"]},
        {"name": "EXEGGUTOR", "moves": ["PSYCHIC", "SLEEP_POWDER", "EXPLOSION", "MEGA_DRAIN"]},
        {"name": "CHANSEY", "moves": ["ICE_BEAM", "THUNDER_WAVE", "SOFT_BOILED", "THUNDERBOLT"]},
        {"name": "ALAKAZAM", "moves": ["PSYCHIC", "THUNDER_WAVE", "RECOVER", "REFLECT"]},
        {"name": "JYNX", "moves": ["BLIZZARD", "PSYCHIC", "LOVELY_KISS", "REST"]},
    ],
    # Stall team
    [
        {"name": "CHANSEY", "moves": ["ICE_BEAM", "THUNDER_WAVE", "SOFT_BOILED", "SEISMIC_TOSS"]},
        {"name": "SNORLAX", "moves": ["BODY_SLAM", "EARTHQUAKE", "REST", "AMNESIA"]},
        {"name": "EXEGGUTOR", "moves": ["PSYCHIC", "SLEEP_POWDER", "STUN_SPORE", "MEGA_DRAIN"]},
        {"name": "ALAKAZAM", "moves": ["PSYCHIC", "RECOVER", "THUNDER_WAVE", "SEISMIC_TOSS"]},
        {"name": "STARMIE", "moves": ["SURF", "BLIZZARD", "THUNDER_WAVE", "RECOVER"]},
        {"name": "RHYDON", "moves": ["EARTHQUAKE", "ROCK_SLIDE", "BODY_SLAM", "SUBSTITUTE"]},
    ],
    # Hyper Offense
    [
        {"name": "TAUROS", "moves": ["BODY_SLAM", "HYPER_BEAM", "EARTHQUAKE", "BLIZZARD"]},
        {"name": "SNORLAX", "moves": ["BODY_SLAM", "HYPER_BEAM", "EARTHQUAKE", "SELF_DESTRUCT"]},
        {"name": "EXEGGUTOR", "moves": ["PSYCHIC", "SLEEP_POWDER", "EXPLOSION", "DOUBLE_EDGE"]},
        {"name": "ALAKAZAM", "moves": ["PSYCHIC", "THUNDER_WAVE", "REFLECT", "SEISMIC_TOSS"]},
        {"name": "GENGAR", "moves": ["THUNDERBOLT", "EXPLOSION", "PSYCHIC", "HYPNOSIS"]},
        {"name": "ZAPDOS", "moves": ["THUNDERBOLT", "DRILL_PECK", "THUNDER_WAVE", "AGILITY"]},
    ],
    # Physical spam
    [
        {"name": "TAUROS", "moves": ["BODY_SLAM", "HYPER_BEAM", "EARTHQUAKE", "BLIZZARD"]},
        {"name": "SNORLAX", "moves": ["BODY_SLAM", "EARTHQUAKE", "HYPER_BEAM", "REST"]},
        {"name": "RHYDON", "moves": ["EARTHQUAKE", "ROCK_SLIDE", "BODY_SLAM", "SUBSTITUTE"]},
        {"name": "EXEGGUTOR", "moves": ["PSYCHIC", "SLEEP_POWDER", "STUN_SPORE", "EXPLOSION"]},
        {"name": "CHANSEY", "moves": ["ICE_BEAM", "THUNDER_WAVE", "SOFT_BOILED", "SEISMIC_TOSS"]},
        {"name": "LAPRAS", "moves": ["SURF", "BLIZZARD", "THUNDERBOLT", "BODY_SLAM"]},
    ],
    # Special spam
    [
        {"name": "ALAKAZAM", "moves": ["PSYCHIC", "THUNDER_WAVE", "RECOVER", "SEISMIC_TOSS"]},
        {"name": "CHANSEY", "moves": ["ICE_BEAM", "THUNDERBOLT", "SOFT_BOILED", "THUNDER_WAVE"]},
        {"name": "EXEGGUTOR", "moves": ["PSYCHIC", "SLEEP_POWDER", "DOUBLE_EDGE", "EXPLOSION"]},
        {"name": "STARMIE", "moves": ["SURF", "BLIZZARD", "THUNDER_WAVE", "RECOVER"]},
        {"name": "JYNX", "moves": ["BLIZZARD", "PSYCHIC", "LOVELY_KISS", "REST"]},
        {"name": "TAUROS", "moves": ["BODY_SLAM", "HYPER_BEAM", "EARTHQUAKE", "BLIZZARD"]},
    ],
    # Reflect Alakazam team
    [
        {"name": "ALAKAZAM", "moves": ["PSYCHIC", "REFLECT", "RECOVER", "THUNDER_WAVE"]},
        {"name": "TAUROS", "moves": ["BODY_SLAM", "HYPER_BEAM", "EARTHQUAKE", "BLIZZARD"]},
        {"name": "CHANSEY", "moves": ["ICE_BEAM", "SOFT_BOILED", "SEISMIC_TOSS", "THUNDER_WAVE"]},
        {"name": "SNORLAX", "moves": ["BODY_SLAM", "EARTHQUAKE", "REST", "SELF_DESTRUCT"]},
        {"name": "EXEGGUTOR", "moves": ["PSYCHIC", "SLEEP_POWDER", "EXPLOSION", "STUN_SPORE"]},
        {"name": "GOLEM", "moves": ["EARTHQUAKE", "ROCK_SLIDE", "BODY_SLAM", "EXPLOSION"]},
    ],
    # Cloyster team
    [
        {"name": "CLOYSTER", "moves": ["CLAMP", "BLIZZARD", "SURF", "EXPLOSION"]},
        {"name": "TAUROS", "moves": ["BODY_SLAM", "HYPER_BEAM", "EARTHQUAKE", "BLIZZARD"]},
        {"name": "EXEGGUTOR", "moves": ["PSYCHIC", "SLEEP_POWDER", "STUN_SPORE", "EXPLOSION"]},
        {"name": "CHANSEY", "moves": ["ICE_BEAM", "THUNDER_WAVE", "SOFT_BOILED", "SEISMIC_TOSS"]},
        {"name": "ALAKAZAM", "moves": ["PSYCHIC", "THUNDER_WAVE", "RECOVER", "SEISMIC_TOSS"]},
        {"name": "RHYDON", "moves": ["EARTHQUAKE", "ROCK_SLIDE", "BODY_SLAM", "SUBSTITUTE"]},
    ],
    # Amnesia Slowbro team
    [
        {"name": "SLOWBRO", "moves": ["AMNESIA", "SURF", "REST", "THUNDER_WAVE"]},
        {"name": "TAUROS", "moves": ["BODY_SLAM", "HYPER_BEAM", "EARTHQUAKE", "BLIZZARD"]},
        {"name": "EXEGGUTOR", "moves": ["PSYCHIC", "SLEEP_POWDER", "EXPLOSION", "STUN_SPORE"]},
        {"name": "CHANSEY", "moves": ["ICE_BEAM", "SOFT_BOILED", "SEISMIC_TOSS", "THUNDER_WAVE"]},
        {"name": "ALAKAZAM", "moves": ["PSYCHIC", "RECOVER", "THUNDER_WAVE", "SEISMIC_TOSS"]},
        {"name": "STARMIE", "moves": ["SURF", "BLIZZARD", "THUNDER_WAVE", "RECOVER"]},
    ],
    # Electrode lead team
    [
        {"name": "ELECTRODE", "moves": ["THUNDERBOLT", "THUNDER_WAVE", "EXPLOSION", "THUNDER"]},
        {"name": "TAUROS", "moves": ["BODY_SLAM", "HYPER_BEAM", "EARTHQUAKE", "BLIZZARD"]},
        {"name": "SNORLAX", "moves": ["BODY_SLAM", "EARTHQUAKE", "HYPER_BEAM", "REST"]},
        {"name": "EXEGGUTOR", "moves": ["PSYCHIC", "SLEEP_POWDER", "STUN_SPORE", "EXPLOSION"]},
        {"name": "CHANSEY", "moves": ["ICE_BEAM", "SOFT_BOILED", "THUNDER_WAVE", "SEISMIC_TOSS"]},
        {"name": "ALAKAZAM", "moves": ["PSYCHIC", "RECOVER", "THUNDER_WAVE", "SEISMIC_TOSS"]},
    ],
    # Jolteon team
    [
        {"name": "JOLTEON", "moves": ["THUNDERBOLT", "THUNDER_WAVE", "PIN_MISSILE", "DOUBLE_KICK"]},
        {"name": "TAUROS", "moves": ["BODY_SLAM", "HYPER_BEAM", "EARTHQUAKE", "BLIZZARD"]},
        {"name": "CHANSEY", "moves": ["ICE_BEAM", "SOFT_BOILED", "THUNDER_WAVE", "SEISMIC_TOSS"]},
        {"name": "EXEGGUTOR", "moves": ["PSYCHIC", "SLEEP_POWDER", "EXPLOSION", "STUN_SPORE"]},
        {"name": "SNORLAX", "moves": ["BODY_SLAM", "EARTHQUAKE", "REST", "AMNESIA"]},
        {"name": "STARMIE", "moves": ["SURF", "BLIZZARD", "THUNDER_WAVE", "RECOVER"]},
    ],
    # Persian team
    [
        {"name": "PERSIAN", "moves": ["SLASH", "HYPER_BEAM", "BUBBLE_BEAM", "THUNDERBOLT"]},
        {"name": "TAUROS", "moves": ["BODY_SLAM", "HYPER_BEAM", "EARTHQUAKE", "BLIZZARD"]},
        {"name": "CHANSEY", "moves": ["ICE_BEAM", "SOFT_BOILED", "THUNDER_WAVE", "SEISMIC_TOSS"]},
        {"name": "EXEGGUTOR", "moves": ["PSYCHIC", "SLEEP_POWDER", "STUN_SPORE", "EXPLOSION"]},
        {"name": "ALAKAZAM", "moves": ["PSYCHIC", "RECOVER", "THUNDER_WAVE", "SEISMIC_TOSS"]},
        {"name": "RHYDON", "moves": ["EARTHQUAKE", "ROCK_SLIDE", "BODY_SLAM", "SUBSTITUTE"]},
    ],
]

def generate_header_file():
    """Generate the C header file with sample teams."""
    
    output = []
    output.append("// Sample RBY OU teams for testing and benchmarking")
    output.append("// Auto-generated from well-known competitive teams")
    output.append("#ifndef SAMPLE_GEN1_OU_TEAMS_H")
    output.append("#define SAMPLE_GEN1_OU_TEAMS_H")
    output.append("")
    output.append('#include "poke_enum.h"')
    output.append('#include "generated_move_enum.h"')
    output.append("")
    output.append("// Structure for a single Pokemon in a team")
    output.append("typedef struct {")
    output.append("  POKEDEX_IDS species;")
    output.append("  MOVE_IDS moves[4];")
    output.append("} TeamPokemon;")
    output.append("")
    output.append("// Structure for a full team of 6 Pokemon")
    output.append("typedef struct {")
    output.append("  TeamPokemon pokemon[6];")
    output.append("} Team;")
    output.append("")
    output.append(f"// Total number of sample teams: {len(SAMPLE_TEAMS)}")
    output.append(f"#define NUM_SAMPLE_TEAMS {len(SAMPLE_TEAMS)}")
    output.append("")
    output.append("static const Team SAMPLE_GEN1_OU_TEAMS[] = {")
    
    for team_idx, team in enumerate(SAMPLE_TEAMS):
        output.append(f"  // Team {team_idx + 1}")
        output.append("  {")
        output.append("    .pokemon = {")
        for poke_idx, pokemon in enumerate(team):
            species = pokemon["name"]
            # Create a copy of moves list to avoid modifying original
            moves = pokemon["moves"].copy()
            # Pad moves to 4 if less
            while len(moves) < 4:
                moves.append("NO_MOVE")
            
            output.append("      {")
            output.append(f"        .species = {species},")
            output.append(f"        .moves = {{{', '.join([f'{m}_MOVE_ID' for m in moves])}}}")
            if poke_idx < len(team) - 1:
                output.append("      },")
            else:
                output.append("      }")
        output.append("    }")
        if team_idx < len(SAMPLE_TEAMS) - 1:
            output.append("  },")
        else:
            output.append("  }")
    
    output.append("};")
    output.append("")
    output.append("#endif // SAMPLE_GEN1_OU_TEAMS_H")
    
    return "\n".join(output)

def main():
    """Main function."""
    header_content = generate_header_file()
    
    # Write to the data_sim directory using relative path
    script_dir = os.path.dirname(os.path.abspath(__file__))
    output_path = os.path.join(script_dir, '..', 'data_sim', 'sample_gen1_ou_teams.h')
    output_path = os.path.normpath(output_path)
    
    with open(output_path, "w") as f:
        f.write(header_content)
    
    print(f"Generated {output_path}")
    print(f"Total teams: {len(SAMPLE_TEAMS)}")
    print("\nTeam summaries:")
    for i, team in enumerate(SAMPLE_TEAMS):
        print(f"  Team {i+1}: {', '.join([p['name'] for p in team])}")

if __name__ == "__main__":
    main()
