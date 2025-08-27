#ifndef BATTLE_QUEUE_H
#define BATTLE_QUEUE_H

//Structs in these files should all be pointers and can be forward declared
#include "stdlib.h"
#include "switch.h"

/**
Basic idea: player swings, moves collide (based on priority/speed), and then
follow up triggers hit the stack. Less important for Gen1.
**/

// Forward declarations
void invalidate_queue(int completed, battlequeue* queue);
int attack(Battle* b, BattlePokemon* attacker, BattlePokemon* defender, Move* used_move);

// Sourced from sort
// negative means a2 should be first, positive means a1 first.
int cmp_priority_qsort(const void* a, const void* b) {
  Action a1 = *(const Action*)a;
  Action a2 = *(const Action*)b;
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

// Both of these functions are used once: inlined
inline int check_tie(Action* a1, Action* a2) {
  return a1->order - a2->order || a1->priority - a2->priority ||
         a1->speed - a2->speed;
}

inline void fischer_yates(Action* arr, int end) {
  Action buff;
  for (int i = end - 1; i > 0; i--) {
    int j = rand() % (i + 1);
    buff = arr[i];
    arr[i] = arr[j];
    arr[j] = buff;
  }
}

// Smallish array: just using builtin sort
// fischer yates shuffler for ties.
inline void sort_queue(battlequeue* bqueue) {
  // There is definitely a faster implementation somewhere
  // Sort is also unstable (though we're randomizing ties so maybe that's not an
  // issue?)
  qsort(bqueue->queue, bqueue->q_size, sizeof(Action), cmp_priority_qsort);

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
//Core queue function: self explanatory. Returns REGULAR if no pokemon have fainted.
// Returns 1 if p1's pokemon have fainted
//Returns 2 if p2's pokemon have fainted
// returns 3 if both pokemon have fained
int eval_queue(Battle* b) {
  for (int i = 0; i < b->action_queue.q_size; i++) {
    Action* current_action = &b->action_queue.queue[i];

    if (current_action->action_type == move_action) {
      Move* move = &current_action->action_d.m;
      attack(b, &current_action->User->active_pokemon, &current_action->Target->active_pokemon, move);

      if (current_action->Target->active_pokemon.pokemon->hp <= 0)
        DLOG("%s Fainted!", get_pokemon_name(b->p1.active_pokemon.pokemon->id));
      // Clear the currently active pokemon
      b->p1.active_pokemon = (BattlePokemon){0};
      b->p1.active_pokemon_index = -1;
      // Handle p1's active pokemon fainting
      if (current_action->User->active_pokemon.pokemon->hp <= 0)
        DLOG("The Opposing %s Fainted!", get_pokemon_name(b->p2.active_pokemon.pokemon->id));
      // Clear the currently active pokemon
      b->p2.active_pokemon = (BattlePokemon){0};
      b->p2.active_pokemon_index = -1;
    } else {
      perform_switch_action(current_action);
    }
    if (b->p1.active_pokemon_index < 0 || b->p2.active_pokemon_index < 0) {
      invalidate_queue(i, &b->action_queue);
      //Fun little trick, I guess? Might need some explaining.
      return -1 * min(b->p1.active_pokemon_index, 0) + -2 * min(b->p2.active_pokemon_index, 0);
      //Early exit for next queue inputs.
    }
  }
  return 0;
}

// Remove everything already completed, and then everything that isn't necessary.
inline void invalidate_queue(int completed, battlequeue* queue) {
  //shouldn't be too uncommon and would save quite a bit of time
  //no need to iterate over anything if everything is already completed!
  if(completed + 1 == queue->q_size) {
    queue->q_size = 0;
    return;
  }
  // First pass: mark invalid actions
  for(int i = 0; i <= completed; i++){
  DLOG("Action has already been completed: removing.", i);
   Action* a = &queue->queue[i];
   a->origLoc = -1; 
  }
  for (int i = completed+1; i < queue->q_size; i++) {
    Action* a = &queue->queue[i];
    if (a->User->active_pokemon_index != a->origLoc) {
      DLOG("Marking %d for pruning", i);
       a->origLoc = -1;
    }
  }
  // Second pass: compact valid actions
  int j = 0;
  for (int i = 0; i < queue->q_size; i++) {
    if (queue->queue[i].origLoc != -1) {
      if (i != j) {
        queue->queue[j] = queue->queue[i];
      }
      j++;
    }
  }
  // Beware: Queue will have invalid elements beyond q_size. This should not be
  // a problem though.
  queue->q_size = j;
}

#endif
