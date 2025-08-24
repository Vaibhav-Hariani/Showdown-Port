#ifndef BASIC_TYPES_H
#define BASIC_TYPES_H

#include "typing.h"
#include "poke_enum.h"
#include "generated_move_enum.h"
#include "stdbool.h"

#include "pokemon.h"
#include "move.h"

// Forward declarations for all major structs
struct STR_BATTLE;
struct STR_PLAYER;
struct STR_ACTION;
struct STR_BQUEUE;

// Typedefs for convenience
typedef struct STR_BATTLE Battle;
typedef struct STR_PLAYER Player;
typedef struct STR_ACTION Action;
typedef struct STR_BQUEUE battlequeue;

// Complete struct definitions

// Battle queue related enums and structs
enum ACTION_MODES {REGULAR, FAINTED};

enum ENUM_ACTIONS {
  move_action,
  switch_action,
  team_action,
  field_action
} typedef action_types;

union UN_ACTIONS {
  Move m;
  int switch_target;
} typedef action_union;

struct STR_ACTION {
  action_union action_d;
  action_types action_type;
  Player* User;
  Player* Target;
  int order;
  int priority;
  int speed;
  int origLoc;
};

struct STR_BQUEUE {
  Action queue[15];
  int q_size;
};

struct STR_POKE {
  unsigned char id;
  Move poke_moves[4];
  poke_stats stats;
  struct STR_STATUS_FLAGS status;
  int hp;
  int max_hp;
  TYPE type1;
  TYPE type2;
};

struct STR_BATTLE_POKEMON {
  Pokemon* pokemon;
  struct STR_STAT_MODS stat_mods;
  TYPE type1;
  TYPE type2;
  int badly_poisoned_ctr;
  int recharge_counter;
  Move recharge_src;
  int recharge_len;
  int dmg_counter;
  int flinch: 1;
  int confusion_counter: 2;
};

struct STR_PLAYER {
  Pokemon team[6];
  BattlePokemon active_pokemon;
  char active_pokemon_index;
};

struct STR_BATTLE {
  int seed;
  Player p1;
  Player p2;
  battlequeue action_queue;
  int turn_num;
  Move* lastMove;
  int lastDamage;
};

#endif
