#ifndef BATTLE_H
#define BATTLE_H

#include <stddef.h>

#include "battle_structs.h"
#include "log.h"
#include "utils.h"

// Define number of Pokemon per team
#define NUM_POKE 1

Player* get_player(Battle* b, int i) { return (i == 1) ? &b->p1 : &b->p2; }
// Forward declarations for functions defined elsewhere
void force_switch(Player* p);

// Any triggers that need to be resolved for inactive pokemon resolve here,
// and the queue is reset to zero.
int end_step(Battle* b) {
  int ret = 0;
  for (int i = 1; i <= 2; i++) {
    Player* p = get_player(b, i);
    DLOG("Player %d's Resolution", i);
    for (int j = 0; j < NUM_POKE; j++) {
      Pokemon* poke = &p->team[j];
      int is_active = (j == p->active_pokemon_index);
      // Sleep
      if (poke->status.sleep) {
        p->active_pokemon.sleep_ctr--;
        if (p->active_pokemon.sleep_ctr == 0) {
          p->active_pokemon.pokemon->status.sleep = 0;
          DLOG("%s woke up!", get_pokemon_name(poke->id));
        } else {
          DLOG("%s is fast asleep.", get_pokemon_name(poke->id));
        }
      }
      // Poison damage
      if (poke->status.poison) {
        int dmg = poke->max_hp / 16;
        poke->hp -= dmg;
        DLOG("%s took poison damage (%d HP)", get_pokemon_name(poke->id), dmg);
      }
      // Badly poisoned damage (active only)
      if (is_active && p->active_pokemon.badly_poisoned_ctr > 0) {
        int dmg = (p->active_pokemon.badly_poisoned_ctr * poke->max_hp) / 16;
        dmg = min(dmg, 15 * poke->max_hp / 16);
        poke->hp -= dmg;
        p->active_pokemon.badly_poisoned_ctr++;
        DLOG("%s took badly poisoned damage (%d HP)",
             get_pokemon_name(poke->id),
             dmg);
      }
      // Burn damage
      if (poke->status.burn) {
        int dmg = poke->max_hp / 16;
        poke->hp -= dmg;
        DLOG("%s took burn damage (%d HP)", get_pokemon_name(poke->id), dmg);
      }
      // Leech seed damage
      if (p->active_pokemon.leech_seed) {
        int dmg =
            (max(p->active_pokemon.badly_poisoned_ctr, 1) * poke->max_hp) / 16;
        poke->hp = max(poke->hp - dmg, 0);
        if (poke->hp > 0) {
          get_player(b, i == 1 ? 2 : 1)->active_pokemon.pokemon->hp +=
              dmg;  // Heal the other player
        }
      }
      // Multi-turn move counter decrement
      if (is_active && p->active_pokemon.multi_move_len > 0) {
        p->active_pokemon.multi_move_len--;
        if (p->active_pokemon.multi_move_len == 0) {
          // Move ended, clear immobilized flag on opponent
          Player* opponent = get_player(b, i == 1 ? 2 : 1);
          opponent->active_pokemon.immobilized = 0;
          p->active_pokemon.multi_move_src = NULL;
        }
      }
      // disabled
      if (p->active_pokemon.disabled_count > 0) {
        p->active_pokemon.disabled_count--;
      }
      // Fainting and active checks
      if (poke->hp <= 0) {
        poke->hp = 0;
        DLOG("%s fainted!", get_pokemon_name(poke->id));
        if (is_active) {
          ret += i;
          // No need to clear volatile effects if fainted
        }
      }
    }
  }
  return ret;
}

#endif