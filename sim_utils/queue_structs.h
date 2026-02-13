#ifndef QUEUE_STRUCTS_H
#define QUEUE_STRUCTS_H

#include <stdint.h>
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
  uint8_t switch_target;  // Changed from int to uint8_t (max 6 Pokemon)
} typedef action_union;

struct STR_ACTION {
  action_union action_d;
  action_types action_type;
  Player* User;
  Player* Target;
  int16_t order;        // Changed from int to int16_t
  int8_t priority;      // Changed from int to int8_t (range -7 to +5)
  int16_t speed;        // Changed from int to int16_t (max speed ~500)
  uint8_t origLoc;      // Changed from int to uint8_t (max 6 locations)
} typedef Action;

// Maximum number of queued actions during a single turn. Original engine
// used a larger queue but, for Gen1/simplified battles, there are at most
// two actions (one per player) per resolution. Shrinking to a size of 2
// enables simpler ordering logic and reduces memory pressure.
#define ACTION_QUEUE_MAX 2
struct STR_BQUEUE {
  Action queue[ACTION_QUEUE_MAX];
  uint8_t q_size;  // Changed from int to uint8_t (max 2)
};

#endif
