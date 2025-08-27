#include "battle.h"

static inline int min(int a, int b) { return (a < b) ? a : b; }

inline Player* get_player(Battle* b, int i) { return (i == 1) ? b->p1 : b->p2; }

void force_switch() {}

// Any triggers that need to be resolved for inactive pokemon resolve here,
// and the queue is reset to zero.
void end_step(Battle* b) {
  // Gen 1 end step: resolve for both active pokemon
  for (int i = 1; i <= 2; i++) {
    Player* p = get_player(b, i);
    BattlePokemon* bp = p->active_pokemon;
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
      dmg = min(dmg, 15 * poke->max_hp / 16);  // Maximum of 15/16ths
      poke->hp -= dmg;
      bp->badly_poisoned_ctr++;
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
    // Check for faint
    if (poke->hp <= 0) {
      poke->hp = 0;
      DLOG("%s fainted!", get_pokemon_name(poke->id));
      // You may want to trigger forced switch logic here
      force_switch();
    }
    // Clear volatile effects (example: flinch, partial trapping)
    bp->recharge_counter = 0;
    bp->recharge_len = 0;
    bp->flinch = 0;  // Clear flinch status at end of turn
    // Add more volatile clears as needed
  }
  // Reset the action queue
  b->action_queue->q_size = 0;
  DLOG("Action queue reset for next turn.");
}