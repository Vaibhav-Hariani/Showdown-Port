#ifndef POKEMON_H
#define POKEMON_H
#include "move.h"

struct STR_STATS {
  unsigned char stats[5];

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
  
  // Currently a percentage loss.
  int stat_mods[5];
  struct STR_STAT_FLAGS status;  
  // Bitfield set by above
} typedef modifiers;

// Comparable to the showdown PokemonSet object, but also storing battle data.
struct STR_POKE {
  // Corresponds to pokedex entryi
  unsigned char id;
  move poke_moves[4];
  // EVs and IVs combined with base stats for compression
  poke_stats stat;

  // Relevant to battle state
  modifiers mods;
  int hp;

} typedef pokemon;




#endif