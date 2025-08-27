#ifndef BASIC_TYPES_H
#define BASIC_TYPES_H

#include "move_fwd.h"
#include "pokemon.h"

// Complete struct definitions

enum ENUM_ACTIONS {
  move_action,
  switch_action,
  team_action,
  field_action
} typedef action_types;

struct STR_BQUEUE {
  Action *queue[15];
  int q_size;
} typedef BattleQueue;

struct STR_PLAYER {
  Pokemon *team[6];
  BattlePokemon *active_pokemon;
  char active_pokemon_index;
} typedef Player;

struct STR_BATTLE {
  int seed;
  Player *p1;
  Player *p2;
  BattleQueue *action_queue;
  int turn_num;
  Move *lastMove;
  int lastDamage;
} typedef Battle;

#endif
