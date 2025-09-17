#ifndef BATTLE_H
#define BATTLE_H

#include <stddef.h>

#include "battle_structs.h"
#include "log.h"
#include "utils.h"

Player* get_player(Battle* b, int i) { return (i == 1) ? &b->p1 : &b->p2; }
// Forward declarations for functions defined elsewhere
void force_switch(Player* p);

// Any triggers that need to be resolved for inactive pokemon resolve here,
// and the queue is reset to zero.

// This is super
int end_step(Battle* b) {
  int ret = 0;
  for (int i = 1; i <= 2; i++) {
    Player* p = get_player(b, i);
    DLOG("Player %d's Resolution", i);
    for (int j = 0; j < 6; j++) {
      Pokemon* poke = &p->team[j];
      int is_active = (j == p->active_pokemon_index);
      int fainted = 0;
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
        DLOG("%s took badly poisoned damage (%d HP)", get_pokemon_name(poke->id), dmg);
      }
      // Burn damage
      if (poke->status.burn) {
        int dmg = poke->max_hp / 16;
        poke->hp -= dmg;
        DLOG("%s took burn damage (%d HP)", get_pokemon_name(poke->id), dmg);
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