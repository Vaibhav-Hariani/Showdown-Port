// switch.h: Handles switching logic and multiple switch types
#ifndef SWITCH_H
#define SWITCH_H

#include "battle_structs.h"
#include "log.h"
#include "string.h"

int valid_switch(Player cur, int target_loc) {
  if (cur.team[target_loc].hp <= 0) {
    DLOG("Switch Ignored: Target Pokémon has fainted.");
    return 0;
  }
  if (cur.team[target_loc].id == MISSINGNO) {
    DLOG("Switch Ignored: Target Pokémon is MISSINGNO.");
    return 0;
  }

  if (cur.active_pokemon_index == target_loc) {
    DLOG("Switch ignored: Pokémon already active.");
    return 0;
  }
  if (cur.active_pokemon.pokemon != NULL &&
      cur.active_pokemon.pokemon->hp > 0 &&
      cur.active_pokemon.no_switch != SWITCH_STOP_NONE) {
    const char* name = get_pokemon_name(cur.active_pokemon.pokemon->id);
    DLOG("Switch Ignored: %s locked in!", name);
    return 0;
  }
  return 1;
}

void add_switch(Battle* b, Player* user, int target_loc, int type) {
  if (b->action_queue.q_size >= ACTION_QUEUE_MAX) {
    DLOG("Action queue full, cannot add switch action.");
    return;
  }
  Action* cur_action = (b->action_queue.queue) + b->action_queue.q_size;
  // memset(cur_action, 0, sizeof(Action));
  cur_action->action_type = switch_action;
  cur_action->User = user;
  cur_action->action_d.switch_target = target_loc;
  cur_action->origLoc = user->active_pokemon_index;
  cur_action->priority = 0;

  // These numbers are very weird: sourced from showdown directly
  cur_action->order = (type == REGULAR) ? 103 : 6;
}

void perform_switch_action(Battle* battle, Action* current_action) {
  int target = current_action->action_d.switch_target;
  Player* user = current_action->User;
  Player* opponent = &battle->p1;
  if (user == &battle->p1) {
    opponent = &battle->p2;
  }
  int old_active = user->active_pokemon_index;

  // Clear opponent's immobilized flag if the switching Pokemon was using a
  // multi-turn move
  if (old_active >= 0 && user->active_pokemon.multi_move_len > 0 &&
      opponent->active_pokemon.immobilized) {
    opponent->active_pokemon.immobilized = 0;
    DLOG("%s was freed from its bindings!",
         get_pokemon_name(opponent->active_pokemon.pokemon->id));
  }

  // Clear user's immobilized flag if they were trapped (switching out frees
  // them)
  if (old_active >= 0 && user->active_pokemon.immobilized) {
    user->active_pokemon.immobilized = 0;
    // Also clear the opponent's multi-turn move
    opponent->active_pokemon.multi_move_len = 0;
    opponent->active_pokemon.multi_move_src = NULL;
  }

  user->shown_pokemon |= (1 << target);

  // Before clearing the old active pokemon, copy revealed flags back to base
  // pokemon
  if (old_active >= 0) {
    for (int i = 0; i < 4; ++i) {
      user->team[old_active].poke_moves[i].pp = user->active_pokemon.moves[i].pp;
      user->team[old_active].poke_moves[i].revealed =
          user->active_pokemon.moves[i].revealed;
    }
  }
  reset_battle_pokemon(&user->active_pokemon);
  user->active_pokemon_index = target;
  user->active_pokemon.pokemon = &user->team[target];
  user->active_pokemon.stats = user->active_pokemon.pokemon->stats;
  user->active_pokemon.type1 = user->active_pokemon.pokemon->type1;
  user->active_pokemon.type2 = user->active_pokemon.pokemon->type2;
  user->active_pokemon.sleep_ctr = user->active_pokemon.pokemon->status.sleep;
  // Copy base Pokemon's moves into the active_pokemon moves array
  memcpy(user->active_pokemon.moves,
         user->active_pokemon.pokemon->poke_moves,
         sizeof(user->active_pokemon.moves));

  if (old_active >= 0 && user->team[old_active].hp > 0) {
    DLOG("Come back %s! ", get_pokemon_name(user->team[old_active].id));
  }
  DLOG("Go %s!", get_pokemon_name(user->active_pokemon.pokemon->id));
}

#endif