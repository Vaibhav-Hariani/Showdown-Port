#ifndef BATTLE_QUEUE_H
#define BATTLE_QUEUE_H

// Structs in these files should all be pointers and can be forward declared
#include "battle_structs.h"
#include "stdlib.h"
#include "switch.h"
/**
Basic idea: player swings, moves collide (based on priority/speed), and then
follow up triggers hit the stack. Less important for Gen1.
**/

// Forward declarations
void invalidate_queue(int completed, battlequeue* queue);
int attack(Battle* b,
           BattlePokemon* attacker,
           BattlePokemon* defender,
           Move* used_move);

// Return true if A goes first. Else, B goes first
//Ordering should be: lower order, higher priority, higher speed should be first. 
inline int gen1_cmp(Action* a, Action* b) {
  int first = a->order - b->order;
  if (first != 0) return first < 0;
  first = a->priority - b->priority;
  if (first != 0) return first > 0;
  first = a->speed - b->speed;
  if (first == 0) {
    // Tie: no need for fischer yates with 2 elements
    return rand() % 2;
  }
  return first > 0;
}

// Gen1-specific re-order, as we will only have two elements at most
void sort_gen1(battlequeue* bqueue) {
  // Swap if the ordering is incorrect
  if (!gen1_cmp(&bqueue->queue[0], &bqueue->queue[1])) {
    Action tmp = bqueue->queue[0];
    bqueue->queue[0] = bqueue->queue[1];
    bqueue->queue[1] = tmp;
  }
}

// Core queue function: self explanatory. Returns REGULAR if no pokemon have
// fainted.
//  Returns 1 if p1's pokemon have fainted
// Returns 2 if p2's pokemon have fainted
//  returns 3 if both pokemon have fainted
int eval_queue(Battle* b) {
  // using gen1 sort for eval_queue: should be significantly faster.
  sort_gen1(&b->action_queue);
  // sort_queue(&b->action_queue);
  for (int i = 0; i < b->action_queue.q_size; i++) {
    Action* current_action = &b->action_queue.queue[i];

    if (current_action->action_type == move_action) {
      Move* move = current_action->action_d.m;
      Player* User = current_action->User;
      Player* Target = current_action->Target;
      if (move == NULL) {
        DLOG("Invalid move in queue at position %d", i);
      }

      attack(b, &User->active_pokemon, &Target->active_pokemon, move);

      // Cleanup
      if (User->active_pokemon.pokemon->hp <= 0) {
        DLOG("%s Fainted!", get_pokemon_name(User->active_pokemon.pokemon->id));
        // Clear the currently active pokemon
        User->active_pokemon = (BattlePokemon){0};
        User->active_pokemon_index = -1;

        // Removing lock from bind/wrap if user faints
        Target->active_pokemon.immobilized = 0;
      }

      // Handle opponent active pokemon fainting
      // This should clear the queue!
      if (Target->active_pokemon.pokemon->hp <= 0) {
        DLOG("The Opposing %s Fainted!",
             get_pokemon_name(Target->active_pokemon.pokemon->id));
        // Clear the currently active pokemon
        Target->active_pokemon = (BattlePokemon){0};
        Target->active_pokemon_index = -1;
      }

    } else {
      perform_switch_action(b, current_action);
    }
    if (b->p1.active_pokemon_index < 0 || b->p2.active_pokemon_index < 0) {
      invalidate_queue(i, &b->action_queue);
      // Fun little trick
      // If p1 is -1, return -1 * -1. If p2 is -1, return 2
      return -1 * min(b->p1.active_pokemon_index, 0) +
             -2 * min(b->p2.active_pokemon_index, 0);
      // Early exit for next queue inputs.
    }
  }
  return 0;
}

// For gen1, this should just invalidate the whole queue afterward. 
// Either this is move one and the queue is invalid, or the queue has been empty
inline void invalidate_queue(int completed, battlequeue* queue) {
    queue->q_size = 0;
    return;
}

#endif
