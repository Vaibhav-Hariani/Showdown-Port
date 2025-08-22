// switch.h: Handles switching logic and multiple switch types
#ifndef SWITCH_H
#define SWITCH_H
#include "string.h"
#include "battle_queue.h"
#include "log.h"
#include "pokemon.h"
#include "stdio.h"

enum ACTION_TYPES {REGULAR, FAINTED };

inline int valid_switch(Player cur, int target_loc) {
  if (cur.team[target_loc].hp <= 0) {
    DLOG("Invalid switch: Target Pokémon has fainted.");
    return 0;
  }
  if (cur.active_pokemon_index == target_loc) {
    DLOG("Switch ignored: Pokémon already active.");
    return 0;
  }
  return 1;
}

inline void add_switch(Battle* b, Player* user, int target_loc, int type) {
  Action* cur_action = (b->action_queue.queue) + b->action_queue.q_size;
  memset(cur_action, 0, sizeof(Action));

  cur_action->action_type = switch_action;
  cur_action->User = user;
  cur_action->action_d.switch_target = target_loc;
  cur_action->origLoc = user->active_pokemon_index;
  cur_action->priority = 0;

  // These numbers are very weird: sourced from showdown directly
  if (type == REGULAR) {
    cur_action->order = 103;
  } else {
    cur_action->order = 6;
  }
  return;
}

// Main switch handler
inline void perform_switch_action(Action* current_action) {
  int target = current_action->action_d.switch_target;

  Player* user = current_action->User;

  int old_active = user->active_pokemon_index;
  // Clear the current active pokemon getup
  memset(&user->active_pokemon, 0, sizeof(BattlePokemon));
  user->active_pokemon.pokemon = &user->team[target];
  user->active_pokemon_index = target;
  user->active_pokemon.type1 = user->active_pokemon.pokemon->type1;
  user->active_pokemon.type2 = user->active_pokemon.pokemon->type2;
  //add in array setting over here for move arr if necessary.

  DLOG("Come back %s! \n Go %s!",
       get_pokemon_name(user->team[old_active]),
       get_pokemon_name(user->team[target]));
}

#endif