# RBY OU Sample Teams

This directory contains pre-defined RBY OU (Red/Blue/Yellow Overused) teams for testing, benchmarking, and reinforcement learning applications.

## File: `sample_gen1_ou_teams.h`

Contains an array of 12 competitive RBY OU teams. Each team consists of 6 Pokémon with their movesets.

### Team Archetypes

1. **Standard Balance** - Classic balanced team with Tauros, Chansey, Exeggutor, Snorlax, Alakazam, and Starmie
2. **Offensive** - Hyper-offensive team with Amnesia Snorlax and Jynx
3. **Stall** - Defensive team with multiple recovery options
4. **Hyper Offense** - All-out offense with Gengar and Zapdos
5. **Physical Spam** - Physical attacker focus with Rhydon and Lapras
6. **Special Spam** - Special attacker focus with multiple Psychic-types
7. **Reflect Alakazam** - Team built around defensive Alakazam with Reflect
8. **Cloyster** - Team featuring Cloyster with Clamp
9. **Amnesia Slowbro** - Team built around bulky Slowbro setup
10. **Electrode Lead** - Fast Electrode lead with Thunder Wave support
11. **Jolteon** - Team featuring Jolteon as Electric-type threat
12. **Persian** - Team with Persian as a fast Normal-type option

## Data Structure

### TeamPokemon
```c
typedef struct {
  POKEDEX_IDS species;
  MOVE_IDS moves[4];
} TeamPokemon;
```

### Team
```c
typedef struct {
  TeamPokemon pokemon[6];
} Team;
```

## Usage

Include the header file in your C code:

```c
#include "sample_gen1_ou_teams.h"

// Access a specific team
const Team* team = &SAMPLE_GEN1_OU_TEAMS[0];

// Access a specific Pokémon from a team
POKEDEX_IDS species = team->pokemon[0].species;
MOVE_IDS first_move = team->pokemon[0].moves[0];
```

## Regenerating Teams

If you need to add more teams or modify existing ones:

1. Edit `generators/generate_sample_teams.py`
2. Add or modify teams in the `SAMPLE_TEAMS` list
3. Run the generator:
   ```bash
   cd generators
   python3 generate_sample_teams.py
   ```

## Notes

- All teams follow RBY OU competitive rules
- Teams are based on well-known archetypes from the competitive metagame
- Movesets are legal and competitive-viable
- Teams exclude Mewtwo and Mew (banned in OU)

## Source

These teams are based on common competitive RBY OU archetypes and movesets used in:
- Smogon's RBY OU metagame
- Pokemon Perfect's RBY competitive scene
- Historical tournament teams

While we attempted to scrape the specific URLs mentioned in the issue, domain access restrictions prevented direct scraping. Instead, these teams represent standard competitive RBY OU builds that are widely used and well-documented in the community.
