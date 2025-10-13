#ifndef POKE_STRUCTS_H
#define POKE_STRUCTS_H

#include "../data_sim/poke_enum.h"
#include "move_structs.h"
#include "stdint.h"

typedef enum {
  SWITCH_STOP_NONE=0,
  SWITCH_STOP_RAGE,
  SWITCH_STOP_SOLAR_BEAM
} SWITCH_STOPS;

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

// Size is 8 bits: 1 int
struct STR_STATUS_FLAGS {
  uint8_t paralyzed : 1;
  uint8_t burn : 1;
  uint8_t freeze : 1;
  uint8_t poison : 1;
  uint8_t sleep : 3;
};
//+-7

// Can combine these into 2 16 bit ints (attack/defense/specA/specD,
// speed/acc/eva)
typedef struct STR_STAT_MODS {
  uint8_t attack : 4;
  uint8_t defense : 4;
  uint8_t speed : 4;
  uint8_t specA : 4;
  uint8_t specD : 4;
  uint8_t accuracy : 4;
  uint8_t evasion : 4;
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
  // moves is by default NULL
  // The existence of a move will override the reference from
  // the base Pokemon struct
  // This is mainly useful for transform / mimic
  // We copy over the moves and the
  poke_stats stats;
  Move moves[4];
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
  SWITCH_STOPS no_switch;
  int multi_move_len;        // Remaining turns for multi-turn moves (Bind, Wrap, etc.)
  Move* multi_move_src;      // The multi-turn move being executed
  uint8_t immobilized : 1;   // Target is immobilized by opponent's multi-turn move
  uint8_t flinch : 1;
  uint8_t confusion_counter : 2;
  uint8_t reflect : 1;
  uint8_t light_screen : 1;
  uint8_t mist : 1;
  uint8_t leech_seed : 1;
  uint8_t disabled_count : 3;
  MOVE_IDS disabled_move_id;  // The move ID that is disabled (not the slot index)
  // if not null, then rage is active
  Move* rage;
  Move* last_used;

} BattlePokemon;

void reset_battle_pokemon(BattlePokemon* bp) {
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
  bp->recharge_src = (Move){0};
  bp->no_switch = SWITCH_STOP_NONE;
  bp->multi_move_len = 0;
  bp->multi_move_src = NULL;
  bp->immobilized = 0;
  bp->flinch = 0;
  bp->confusion_counter = 0;
  bp->reflect = 0;
  bp->light_screen = 0;
  bp->mist = 0;
  bp->leech_seed = 0;
  bp->disabled_count = 0;
  bp->disabled_move_id = NO_MOVE;
  bp->rage = NULL;
  bp->last_used = NULL;
}

#endif