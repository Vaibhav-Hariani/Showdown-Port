#ifndef POKEMON_H
#define POKEMON_H
#include "move.h"
#include "pokedex.h"  // For accessing pokemon_base
#include "stdbool.h"
// Add an ENUM for stat labels
typedef enum {
  STAT_HP,
  STAT_ATTACK,
  STAT_DEFENSE,
  STAT_SPECIAL_ATTACK,
  STAT_SPECIAL_DEFENSE,
  STAT_SPEED,
  STAT_COUNT  // Total number of stats
} STAT_LABELS;

// Convert poke_stats to an array-based structure
struct STR_STATS {
  int base_stats[STAT_COUNT];  // Array to hold all base_stats
  int level;
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

// As per Davids idea: sleep turns and length is set here
struct STR_STATUS_FLAGS {
  int paralyzed : 1;
  int burn : 1;
  int freeze : 1;
  int poison : 1;
  int sleep : 3;
  int flinch: 1;
};
//+-7
struct STR_DEBUFF_STATS {
  int attack : 4;
  int defense : 4;
  int speed: 4;
  int specA: 4;
  int specD: 4;
  int accuracy: 4;
  int evasion: 4;
};

// Comparable to the showdown PokemonSet object, but also storing battle data.
struct STR_POKE {
  // Corresponds to pokedex entry
  unsigned char id;
  Move poke_moves[4];
  // EVs and IVs are combined with base_stats for compression
  poke_stats stats;
  // Relevant to battle state
  struct STR_STATUS_FLAGS status;
  int hp;
  //Needed for healing moves & caps
  int max_hp;
  TYPE type1;  // Primary type
  TYPE type2;  // Secondary type
} typedef Pokemon;

struct STR_BATTLE_POKE {
  Pokemon* pokemon;
  //These are wiped whenever the active pokemon is switched in or out
  struct STR_DEBUFF_STATS stat_mods;
  TYPE type1;
  TYPE type2;

  bool flinch;
  //Ticks for how long the pokemon has been badly poisoned for.
  int badly_poisoned_ctr;
  //Used for all multi moves:
  int recharge_counter;
  int recharge_src;
  int recharge_len;
  //Used for bide
  int dmg_counter;

} typedef BattlePokemon;

#endif