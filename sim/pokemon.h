#ifndef POKEMON_H
#define POKEMON_H
#include "move.h"
#include "pokemon_base.h" // For accessing pokemon_base

// Add an ENUM for stat labels
typedef enum {
    STAT_HP,
    STAT_ATTACK,
    STAT_DEFENSE,
    STAT_SPECIAL_ATTACK,
    STAT_SPECIAL_DEFENSE,
    STAT_SPEED,
    STAT_COUNT // Total number of stats
} STAT_LABELS;

// Convert poke_stats to an array-based structure
struct STR_STATS {
    int stats[STAT_COUNT]; // Array to hold all stats
    // Future relevance
    //  bool gender;
    //  int item_id;
    //  int happiness;
    //  int ability_id;
    //  int nature_id;

    // > Gen7 tomfoolery
    // int hp_type;
    // int dmaxlevel;
    // int gmaxlvl;
    // int teratype;
} typedef poke_stats;

struct STR_STAT_FLAGS{
  int paralyzed: 1;
  int burn: 1;
  int freeze: 1;
  int poision: 1;
  int sleep: 1;
};

struct STR_MODS {
  // Move/ability effects that modify stats temporarily: applies on base stats
  // Percentage stat debuff.
  int stat_mods[6];
  struct STR_STAT_FLAGS status;
  // Bitfield set by above
} typedef modifiers;

// Comparable to the showdown PokemonSet object, but also storing battle data.
struct STR_POKE {
  // Corresponds to pokedex entry
  unsigned char id;
  move poke_moves[4];
  // EVs and IVs combined with base stats for compression
  poke_stats stat;

  // Relevant to battle state
  modifiers mods;
  int hp;

  // Add type1 and type2 fields to STR_POKE for Pokémon typing
  TYPE type1; // Primary type
  TYPE type2; // Secondary type
} typedef pokemon;

// Update initialize_pokemon to use a local variable for pokemon_base[id - 1]
pokemon initialize_pokemon(POKEDEX id) {
    pokemon p;
    p.id = id;

    if (id < LAST_POKEMON) {
        const poke_ref *base = &pokemon_base[id - 1]; // Store reference to pokemon_base[id - 1]

        // Initialize stats from pokemon_base
        for (int i = 0; i < STAT_COUNT; i++) {
            p.stat.stats[i] = base->base_stats[i];
        }

        // Set Pokémon types
        p.type1 = base->primary_type;
        p.type2 = base->secondary_type;

        // Apply EVs/IVs logic for Gen1
        int iv = 15; // Max IV for Gen1 (0-15 scale)
        int ev = 65535; // Max EV for Gen1 (0-65535 scale)

        // Calculate HP stat separately
        p.stat.stats[STAT_HP] += (iv * 2 + ev / 4) / 100 + 10;

        // Calculate other stats
        for (int i = 1; i < STAT_COUNT; i++) {
            p.stat.stats[i] += (iv * 2 + ev / 4) / 100 + 5;
        }

        p.hp = p.stat.stats[STAT_HP];
        for (int i = 0; i < 4; i++) {
            p.poke_moves[i] = (i == 0) ? moves[TACKLE] : (move){0}; // Assign Tackle as the first move, others empty
        }
    }
    return p;
}

#endif