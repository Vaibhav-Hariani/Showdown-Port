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
//Todo: add in confusion
//I have no clue how substitute is going to work but we'll figure it out eventually
struct STR_STATUS_FLAGS {
  int paralyzed : 1;
  int burn : 1;
  int freeze : 1;
  int poison : 1;
  int sleep : 3;
 };
//+-7
struct STR_DEBUFF_STATS {
  int attack : 4;
  int defense : 4;
  int speed : 4;
  int specA : 4;
  int specD : 4;
  int accuracy : 4;
  int evasion : 4;
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
  // Needed for healing moves & caps
  int max_hp;
  TYPE type1;  // Primary type
  TYPE type2;  // Secondary type
} typedef Pokemon;


//Ditto (and it's messy messy requirements) could mean that we need a separate active move_array for ditto and ditto alone. 
//However, ditto is also unique in that it copies EVERY aspect of the other pokemon: we could just modify ditto directly and then, when ditto switches out, reset him(?)
struct STR_BATTLE_POKE {
  // These are wiped whenever the active pokemon is switched in or out
  Pokemon* pokemon;
  struct STR_DEBUFF_STATS stat_mods;
  TYPE type1;
  TYPE type2;
  // Ticks for how long the pokemon has been badly poisoned for.
  int badly_poisoned_ctr;
  // Used for all multi moves:
  int recharge_counter;
  //ident for recharge source
  int recharge_src;
  //how long recharging should last 
  int recharge_len;
  // Used for bide
  int dmg_counter;
  //flinch and confusion are temporary.
  int flinch: 1;
  //Confusion can last 3 turns, tops
  int confusion_counter: 3;

} typedef BattlePokemon;

#endif