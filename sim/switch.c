#include "switch.h"

#include <stdio.h>
#include <string.h>

#include "log.h"
#include "move.h"
#include "pokemon.h"

inline int valid_switch(Player* cur, int target_loc) {
  if (cur->team[target_loc]->hp <= 0) {
    DLOG("Invalid switch: Target Pokémon has fainted.");
    return 0;
  }
  if (cur->active_pokemon_index == target_loc) {
    DLOG("Switch ignored: Pokémon already active.");
    return 0;
  }
  return 1;
}

inline void add_switch(Battle* b, Player* user, int target_loc, int type) {
  Action* cur_action = b->action_queue->queue[b->action_queue->q_size];

  reset_action(cur_action);
  set_action_action_d(cur_action, (action_union){.switch_target = target_loc});
  set_action_action_type(cur_action, switch_action);
  set_action_user(cur_action, user);
  set_action_priority(cur_action, 0);
  set_action_orig_loc(cur_action, user->active_pokemon_index);
  // These numbers are very weird: sourced from showdown directly
  set_action_order(cur_action, (type == REGULAR) ? 103 : 6);
}

// Main switch handler
inline void perform_switch_action(Action* current_action) {
  int target = get_action_action_d(current_action).switch_target;
  Player* user = get_action_user(current_action);

  // Clear the current active pokemon getup
  memset(user->active_pokemon, 0, sizeof(BattlePokemon));
  user->active_pokemon->pokemon = user->team[target];
  user->active_pokemon_index = target;
  user->active_pokemon->type1 = user->active_pokemon->pokemon->type1;
  user->active_pokemon->type2 = user->active_pokemon->pokemon->type2;
  // add in array setting over here for move arr if necessary.

#ifdef DEBUG
  int old_active = user->active_pokemon_index;
  DLOG("Come back %s! \n Go %s!",
       get_pokemon_name(user->team[old_active]->id),
       get_pokemon_name(user->team[target]->id));
#endif
}