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
    int base_stats[STAT_COUNT]; // Array to hold all base_stats
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

//As per your idea: sleep turns and length is set here
struct STR_STAT_FLAGS{
  int paralyzed: 1;
  int burn: 1;
  int freeze: 1;
  int poison: 1;
  int sleep: 3;
};

struct STR_MODS {
  // Move/ability effects that modify base_stats temporarily: applies on base base_stats
  int stat_mods[STAT_COUNT];
  struct STR_STAT_FLAGS status;
} typedef modifiers;

// Comparable to the showdown PokemonSet object, but also storing battle data.
struct STR_POKE {
  // Corresponds to pokedex entry
  unsigned char id;
  move poke_moves[4];
  // EVs and IVs are combined with base_stats for compression
  poke_stats stats;

  // Relevant to battle state
  modifiers mods;
  int hp;
  //Read for move damage
  int level;
  // Add type1 and type2 fields to STR_POKE for Pokémon typing
  TYPE type1; // Primary type
  TYPE type2; // Secondary type
} typedef pokemon;

#endif