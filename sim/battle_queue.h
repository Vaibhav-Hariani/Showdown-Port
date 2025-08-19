#ifndef BATTLE_QUEUE_H
#define BATTLE_QUEUE_H

// Trying to re-implement the battle queue from showdown
#include "stdlib.h"
#include "battle.h"
#include "pokemon.h"
#include "move.h"
#include "move_labels.h"
#include "pokedex_labels.h"
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
//   //  Action source_Action: this is included in showdown but unsure about usage
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
  Move m;
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
  Player* p;

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

void eval_queue(Battle *b) {
  for (int i = 0; i < b->action_queue.q_size; i++) {
    action *current_action = &b->action_queue.queue[i];

    if (current_action->action_type == move_action) {
      Move* move = &current_action->action_d.m;
      BattlePokemon attacker_bp, defender_bp;
      
      // attacker_bp.pokemon = &current_action->p->team[current_action->origLoc];
      // defender_bp.pokemon = (current_action->player_num == 1) ? &b->p2.team[b->p2.active_pokemon] : &b->p1.team[b->p1.active_pokemon];
      Move *used_move = &current_action->action_d.m;
      attack(b, &attacker_bp, &defender_bp, used_move);
      if(b->p1.active_pokemon.pokemon.hp <= 0)
        DLOG("Player 1's active Pokémon fainted.");
        // Handle p1's active pokemon fainting
      }

      if (defender_bp.pokemon->hp <= 0) {
        printf("%s fainted!\n", PokemonNames[defender_bp.pokemon->id]);
        int fainted_pokemon_loc = defender_bp.pokemon - current_action->p->team;
        invalidate_queue(&b->action_queue, fainted_pokemon_loc);
        printf("Player %d must switch in a new Pokémon.\n", current_action->player_num);
      }
    } else if (current_action->action_type == switch_action) {
      perform_switch(current_action, &b->action_queue);
    }
  }
}

// Refactored switch logic for readability
void perform_switch(action *current_action, battlequeue *queue) {
  int target = current_action->action_d.switch_target;
  if (current_action->p->team[target].hp <= 0) {
    printf("Invalid switch: Target Pokémon has fainted.\n");
    return;
  }
  if (current_action->p->active_pokemon == target) {
    printf("Switch ignored: Pokémon already active.\n");
    return;
  }
  int old_active = current_action->p->active_pokemon;
  current_action->p->active_pokemon = target;
  printf("Player %d switched to %s!\n", current_action->player_num, PokemonNames[current_action->p->team[target].id]);
  invalidate_queue(queue, old_active);
}

// Add a function to invalidate and shift the queue
void invalidate_queue(battlequeue *queue, int fainted_pokemon_loc) {
    for (int i = 0; i < queue->q_size; i++) {
        if (queue->queue[i].origLoc == fainted_pokemon_loc) {
            printf("Removing invalid action for fainted Pokémon.\n");
            // Shift remaining actions up
            for (int j = i; j < queue->q_size - 1; j++) {
                queue->queue[j] = queue->queue[j + 1];
            }
            queue->q_size--;
            i--; // Recheck the current index
        }
    }
}
#endif