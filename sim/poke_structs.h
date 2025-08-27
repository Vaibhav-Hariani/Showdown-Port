#ifndef POKE_STRUCTS_H
#define POKE_STRUCTS_H

#include "move_structs.h"
// Forward declarations
typedef enum {
  STAT_HP,
  STAT_ATTACK,
  STAT_DEFENSE,
  STAT_SPECIAL_ATTACK,
  STAT_SPECIAL_DEFENSE,
  STAT_SPEED,
  STAT_COUNT  // Total number of stats
} STAT_LABELS;

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
// Todo: add in confusion
// I have no clue how substitute is going to work but we'll figure it out
// eventually
struct STR_STATUS_FLAGS {
  int paralyzed : 1;
  int burn : 1;
  int freeze : 1;
  int poison : 1;
  int sleep : 3;
};
//+-7
typedef struct STR_STAT_MODS {
  int attack : 4;
  int defense : 4;
  int speed : 4;
  int specA : 4;
  int specD : 4;
  int accuracy : 4;
  int evasion : 4;
} stat_mods;

typedef struct STR_POKE {
  unsigned char id;
  Move poke_moves[4];
  poke_stats stats;
  struct STR_STATUS_FLAGS status;
  int hp;
  int max_hp;
  TYPE type1;
  TYPE type2;
} Pokemon;

typedef struct STR_BATTLE_POKEMON {
  Pokemon* pokemon;
  struct STR_STAT_MODS stat_mods;
  TYPE type1;
  TYPE type2;
  int badly_poisoned_ctr;
  int recharge_counter;
  Move recharge_src;
  int recharge_len;
  int dmg_counter;
  int flinch : 1;
  int confusion_counter : 2;
} BattlePokemon;

#endif