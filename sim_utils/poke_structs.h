#ifndef POKE_STRUCTS_H
#define POKE_STRUCTS_H

#include "../data_sim/poke_enum.h"
#include "move_structs.h"
#include "stdint.h"
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
  int base_stats[STAT_COUNT];  // Array to hold all base_stats, post IVs/EVs
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

// I have no clue how substitute is going to work but we'll figure it out
// eventually
// Size is 8 bits: 1 int
struct STR_STATUS_FLAGS {
  char paralyzed : 1;
  char burn : 1;
  char freeze : 1;
  char poison : 1;
  char sleep : 3;
};
//+-7

// Can combine these into 2 16 bit ints (attack/defense/specA/specD,
// speed/acc/eva)
typedef struct STR_STAT_MODS {
  char attack : 4;
  char defense : 4;
  char speed : 4;
  char specA : 4;
  char specD : 4;
  char accuracy : 4;
  char evasion : 4;
} stat_mods;

typedef struct STR_POKE {
  POKEDEX_IDS id;
  Move poke_moves[4];
  poke_stats stats;
  struct STR_STATUS_FLAGS status;
  // Both of these are of length 16
  int16_t hp;
  int16_t max_hp;
  TYPE type1;
  TYPE type2;
} Pokemon;

typedef struct STR_BATTLE_POKEMON {
  Pokemon* pokemon;
  struct STR_STAT_MODS stat_mods;
  TYPE type1;
  TYPE type2;
  int substitute_hp;
  int badly_poisoned_ctr;
  int sleep_ctr;
  int recharge_counter;
  Move recharge_src;
  int recharge_len;
  int dmg_counter;
  int flinch : 1;
  // Todo: Add in confusion return as
  int confusion_counter : 2;
  int reflect : 1;
  int light_screen : 1;
  int mist : 1;

} BattlePokemon;

def reset_battle_pokemon(BattlePokemon* bp) {
  bp->stat_mods.attack = 0;
  bp->stat_mods.defense = 0;
  bp->stat_mods.speed = 0;
  bp->stat_mods.specA = 0;
  bp->stat_mods.specD = 0;
  bp->stat_mods.accuracy = 0;
  bp->stat_mods.evasion = 0;
  bp->badly_poisoned_ctr = 0;
  bp->sleep_ctr = 0;
  bp->recharge_counter = 0;
  bp->recharge_len = 0;
  bp->dmg_counter = 0;
  bp->flinch = 0;
  bp->confusion_counter = 0;
  bp->reflect = 0;
  bp->light_screen = 0;
  bp->mist = 0;
}

#endif