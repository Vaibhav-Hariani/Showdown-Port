#ifndef BATTLE_QUEUE_H
#define BATTLE_QUEUE_H

// Trying to re-implement the battle queue from showdown
#include "stdlib.h"
/**

Basic idea: player swings, moves collide (based on priority/speed), and then
follow up triggers hit the stack. Less important for Gen1.
 **/

// Currently unnecessary
// struct STR_MOVE_ACTION {
//   void* (*move_funct)(battle, pokemon*, pokemon*);
//   // Enums should be used for mega/zmove/maxmoves
//   //  bool mega;
//   //  bool zmove;
//   //  bool maxmove;
//   //  Action source_Action: this is included in showdown but unsure about
//   usage
// };

// Important: switches need to be verified by checking target HP.
struct STR_SWITCH_ACTION {
  int targetLoc;
  // effect that called the switch: not relevant for gen1
  // sourceEffect: Effect | null;
};

enum ENUM_ACTIONS {
  move_action,
  switch_action,
  team_action,
  field_action
} typedef action_types;

union UN_ACTIONS {
  move m;
  int switch_target;
  // struct STR_SWITCH_ACTION s;

  // This may be necessary: It's used for switch-in effects (unsure if any exist
  // in gen1),
  //  as well as more generic event triggers that need to happen dynamically.
  //  struct POKEMON_ACTION p; // Used for dynamaxxing/mega evo
  // struct STR_TEAM_ACTION t;
  // struct STR_FIELD_ACTION m;
} typedef action_union;

struct STR_ACTION {
  action_union action_d;
  action_types action_type;
  // which player is making the move
  int player_num;
  player* p;

  int order;
  int priority;
  // Can probably be pulled from the origin pokemon
  int speed;
  // (index of the pokemon making the move)
  // if not active pokemon, move is cancelled
  int origLoc;

  // This is only used for more complex effects, not determining the end target
  // int targetLoc; // (index of the pokemon being targeted): used for doubles
  // pokemon initiator;
  // pokemon OriginalTarget;
} typedef action;

// The current battle state: according to showdown docs,
// sorted by priority (not midturn) for gen 1-7.
// not a 'true' priority queue
// Setting to 15 so there's no dynamic allocation
struct STR_BQUEUE {
  action queue[15];
  int q_size;
} typedef battlequeue;

// Sourced from sort
// negative means a2 should be first, positive means a1 first.
int cmp_priority_qsort(const void* a, const void* b) {
  action a1 = *(const action*)a;
  action a2 = *(const action*)b;
  int diff = (a1.order) - (a2.order);
  if (!diff) {
    diff = (a1.priority) - (a2.priority);
    if (!diff) {
      diff = (a1.speed) - (a2.speed);
    }
    // Showdown docs have suborder and effectOrder,
    //  I think these can be ignored for gen1
  }
  return diff;
}

// Both of these funcitons are used once: inlined
inline int check_tie(action* a1, action* a2) {
  return a1->order - a2->order || a1->priority - a2->priority ||
         a1->speed - a2->speed;
}

inline void fischer_yates(action* arr, int end) {
  action buff;
  for (int i = end - 1; i > 0; i--) {
    int j = rand() % (i + 1);
    buff = arr[i];
    arr[i] = arr[j];
    arr[j] = buff;
  }
}

// Smallish array: just using builtin sort
// fischer yates shuffler for ties.
void sort_queue(battlequeue* bqueue) {
  // There is definitely a faster implementation somewhere
  // Sort is also unstable (though we're randomizing ties so maybe that's not an
  // issue?)
  qsort(bqueue->queue, bqueue->q_size, sizeof(action), cmp_priority_qsort);

  int j = 0;
  while (j < bqueue->q_size - 1) {
    int buf = 1;
    while (j + buf < bqueue->q_size &&
           !check_tie((bqueue->queue + j), bqueue->queue + j + buf)) {
      buf++;
    }
    fischer_yates(bqueue->queue + j, buf);
    j += buf;
  }
}
#endif