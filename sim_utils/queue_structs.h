#ifndef QUEUE_STRUCTS_H
#define QUEUE_STRUCTS_H

#include "move_structs.h"

// Battle queue related enums and structs
enum ACTION_MODES { REGULAR, FAINTED };

typedef union UN_ACTIONS action_union;

enum ENUM_ACTIONS {
  move_action,
  switch_action,
  team_action,
  field_action
} typedef action_types;

union UN_ACTIONS {
  Move* m;
  // Index of switch target
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
} typedef Action;

struct STR_BQUEUE {
  Action queue[15];
  int q_size;
};

#endif
