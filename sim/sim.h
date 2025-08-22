#ifndef SIM_H
#define SIM_H
#include "battle.h"
#include "battle_queue.h"
#include "move.h"
#include "switch.h"
#include "pokemon.h"

// Input value if valid, zero if selection failed
//Inputs range from 1 to 9;
int action(Battle* b, Player* user, Player* target, int input, int type) {
  //   if (input > 10 ||i input <= 0 || type == FAINTED && input > 6) {
  //   return 0;
  // }
  Action* cur = (b->action_queue.queue) + b->action_queue.q_size;
  b->action_queue.q_size++;
  if (input >= 7 && type != REGULAR) {
    input -= 7;
    return add_move_to_queue(b, user, target, input);
  } else {
    return add_switch(b, user, input, type);
  }
  //Something has failed
  return 0;
}

#endif