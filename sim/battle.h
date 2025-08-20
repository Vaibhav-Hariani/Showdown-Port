// Effectively, the runner.
//  Sourced from battle.ts

#ifndef BATTLE_H
#define BATTLE_H
#include "pokemon.h"
#include "move.h"
#include "battle_queue.h"
// Their battle object contains a loooot of metadata.
// I'm combining battleOptions and battle into one "big" system

// From research
//  Can see other pokemon HP percentage as well as stat percentage
//  Status effects are revealed
//  Cannot see entire team: only pokemon they played already. This can be
//  simulated with missingNo for unknowns?
// For gen1:
// Desync Clause Mod: Desyncs changed to move failure.
// Sleep Clause Mod: Limit one foe put to sleep
// Freeze Clause Mod: Limit one foe frozen
// Species Clause: Limit one of each Pokémon
// OHKO Clause: OHKO moves are banned
// Evasion Moves Clause: Evasion moves are banned
// Endless Battle Clause: Forcing endless battles is banned
// HP Percentage Mod: HP is shown in percentages

struct STR_PLAYER {
  Pokemon team[6];
  BattlePokemon active_pokemon;
  char active_pokemon_index;
} typedef Player;


struct STR_BATTLE {
  int seed;
  Player p1;
  Player p2;
  //Doesn't really exist as a concept (moves are entered into the queue simultaneously)
  // char cur_player;
  // No field, as weather effects weren't a thing in gen1
  battlequeue action_queue;
  // Major modification happening here by moving Move actions outside of a
  // BattleActions class This should streamline things (hopefully)?
  int turn_num;

  // Metadata for stuff like counter
  Move* lastMove;
  int lastDamage;
  // Effects, events, and active pokemon go here
  // Unsure how this should be structured:
  // It seems the architecture relies on these being set, and then handles
  // poison/toxic and other callback loops Leaving them out for now, but may
  // need them in the near future
} typedef Battle;

inline Player* get_player(Battle* b, int i) {
  return (i == 1) ? &b->p1 : &b->p2;
}

// Any triggers that need to be resolved for inactive pokemon resolve here,
// and the queue is reset to zero.
void end_step(Battle* b) {
  // Gen 1 end step: resolve for both active pokemon
  for (int i = 1; i <= 2; i++) {
    Player* p = get_player(b, i);
    DLOG("%s's Resolution", get_player_name(p));
    BattlePokemon* bp = &p->active_pokemon;
    Pokemon* poke = bp->pokemon;
    if (poke == NULL) continue;

    // Poison damage
    if (poke->status.poison) {
      int dmg = poke->max_hp / 16;
      poke->hp -= dmg;
      DLOG("%s took poison damage (%d HP)", get_pokemon_name(poke->id), dmg);
    }
    // Badly poisoned damage
    if (bp->badly_poisoned_ctr > 0) {
      int dmg = (bp->badly_poisoned_ctr * poke->max_hp) / 16;
      dmg = min(dmg, 15 * poke->max_hp / 16); // Minimum of 1/16th
      poke->hp -= dmg;
      bp->badly_poisoned_ctr++;
      DLOG("%s took badly poisoned damage (%d HP)", get_pokemon_name(poke->id), dmg);
    }
    // Burn damage
    if (poke->status.burn) {
      int dmg = poke->max_hp / 16;
      poke->hp -= dmg;
      DLOG("%s took burn damage (%d HP)", get_pokemon_name(poke->id), dmg);
    }
    // Check for faint
    if (poke->hp <= 0) {
      poke->hp = 0;
      DLOG("%s fainted!", get_pokemon_name(poke->id));
      // You may want to trigger forced switch logic here
      force_switch(p);
    }
  // Clear volatile effects (example: flinch, partial trapping)
  bp->recharge_counter = 0;
  bp->recharge_len = 0; 
  // Add more volatile clears as needed
  }
  // Reset the action queue
  b->action_queue.q_size = 0;
  DLOG("Action queue reset for next turn.");
}

#endif