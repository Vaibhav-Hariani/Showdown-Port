#include "battle_queue.h"

#include <stdlib.h>

#include "log.h"
#include "move.h"
#include "pokemon.h"
#include "switch.h"

static inline int min(int a, int b) { return (a < b) ? a : b; }

// Sourced from sort
// negative means a2 should be first, positive means a1 first.
int cmp_priority_qsort(const void* a, const void* b) {
  Action* a1 = (Action*)a;
  Action* a2 = (Action*)b;
  int diff = get_action_order(a1) - get_action_order(a2);
  if (!diff) {
    diff = get_action_priority(a1) - get_action_priority(a2);
    if (!diff) {
      diff = get_action_speed(a1) - get_action_speed(a2);
    }
    // Showdown docs have suborder and effectOrder,
    //  I think these can be ignored for gen1
  }
  return diff;
}

// Both of these functions are used once: inlined
inline int check_tie(Action* a1, Action* a2) {
  return get_action_order(a1) - get_action_order(a2) ||
         get_action_priority(a1) - get_action_priority(a2) ||
         get_action_speed(a1) - get_action_speed(a2);
}

inline void fischer_yates(Action** arr, int end) {
  Action* buff;
  for (int i = end - 1; i > 0; i--) {
    int j = rand() % (i + 1);
    buff = arr[i];
    arr[i] = arr[j];
    arr[j] = buff;
  }
}

// Smallish array: just using builtin sort
// fischer yates shuffler for ties.
inline void sort_queue(BattleQueue* bqueue) {
  // There is definitely a faster implementation somewhere
  // Sort is also unstable (though we're randomizing ties so maybe that's not an
  // issue?)
  qsort(bqueue->queue, bqueue->q_size, get_action_size(), cmp_priority_qsort);

  int j = 0;
  while (j < bqueue->q_size - 1) {
    int buf = 1;
    while (j + buf < bqueue->q_size &&
           !check_tie(bqueue->queue[j], bqueue->queue[j + buf])) {
      buf++;
    }
    fischer_yates(bqueue->queue + j, buf);
    j += buf;
  }
}
// Core queue function: self explanatory. Returns REGULAR if no pokemon have
// fainted.
//  Returns 1 if p1's pokemon have fainted
// Returns 2 if p2's pokemon have fainted
//  returns 3 if both pokemon have fained
inline int eval_queue(Battle* b) {
  for (int i = 0; i < b->action_queue->q_size; i++) {
    Action* current_action = b->action_queue->queue[i];

    if (get_action_action_type(current_action) == move_action) {
      Move* move = get_action_action_d(current_action).m;
      attack(b,
             get_action_user(current_action)->active_pokemon,
             get_action_target(current_action)->active_pokemon,
             move);

      if (get_action_target(current_action)->active_pokemon->pokemon->hp <= 0) {
        DLOG("%s Fainted!",
             get_pokemon_name(get_action_target(current_action)
                                  ->active_pokemon->pokemon->id));
      }

      // Clear the currently active pokemon
      b->p1->active_pokemon = NULL;
      b->p1->active_pokemon_index = -1;
      // Handle p1's active pokemon fainting
      if (get_action_user(current_action)->active_pokemon->pokemon->hp <= 0) {
        DLOG("The Opposing %s Fainted!",
             get_pokemon_name(b->p2->active_pokemon->pokemon->id));
      }

      // Clear the currently active pokemon
      b->p2->active_pokemon = NULL;
      b->p2->active_pokemon_index = -1;
    } else {
      perform_switch_action(current_action);
    }
    if (b->p1->active_pokemon_index < 0 || b->p2->active_pokemon_index < 0) {
      invalidate_queue(i, b->action_queue);
      // Fun little trick, I guess? Might need some explaining.
      return -1 * min(b->p1->active_pokemon_index, 0) +
             -2 * min(b->p2->active_pokemon_index, 0);
      // Early exit for next queue inputs.
    }
  }
  return 0;
}

// Remove everything already completed, and then everything that isn't
// necessary.
inline void invalidate_queue(int completed, BattleQueue* queue) {
  // shouldn't be too uncommon and would save quite a bit of time
  // no need to iterate over anything if everything is already completed!
  if (completed + 1 == queue->q_size) {
    queue->q_size = 0;
    return;
  }
  // First pass: mark invalid actions
  for (int i = 0; i <= completed; i++) {
    DLOG("Action %d has already been completed: removing.", i);
    Action* a = queue->queue[i];
    set_action_orig_loc(a, -1);
  }
  for (int i = completed + 1; i < queue->q_size; i++) {
    Action* a = queue->queue[i];
    if (get_action_user(a)->active_pokemon_index != get_action_orig_loc(a)) {
      DLOG("Marking %d for pruning", i);
      set_action_orig_loc(a, -1);
    }
  }
  // Second pass: compact valid actions
  int j = 0;
  for (int i = 0; i < queue->q_size; i++) {
    if (get_action_orig_loc(queue->queue[i]) != -1) {
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