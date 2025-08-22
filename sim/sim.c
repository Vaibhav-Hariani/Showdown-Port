#include "battle.h"
#include "battle_queue.h"
#include "generated_move_enum.h"
#include "move.h"
#include "poke_enum.h"
#include "pokedex.h"
#include "pokemon.h"
#include "stdio.h"
Battle b = {0};

// ToDo: implement init_battle(), evaluate the queue
// Then, add checks for status effects, misses, etc. etc. Follow OU rules, and
// implement switching mechanics.
int main() {
  // Initialize battle
  Battle b = {0};
  Player p1 = {0};
  Player p2 = {0};
  b.p1 = p1;
  b.p2 = p2;
  return 0;
}

void print_state(Player player) {
  for (int i = 0; i < 6; i++) {
    Pokemon p = player.team[i];
    if (player.active_pokemon_index == i) {
      DLOG("Active Pokemon: #%d: %s \n", i, get_pokemon_name(p.id));
      DLOG(
          "Stat Modifiers: Atk=%d, Def=%d, Spd=%d, Spec=%d, Acc=%d, "
          "Eva=%d \n",
          player.active_pokemon.stat_mods.attack,
          player.active_pokemon.stat_mods.defense,
          player.active_pokemon.stat_mods.speed,
          player.active_pokemon.stat_mods.specA,
          player.active_pokemon.stat_mods.accuracy,
          player.active_pokemon.stat_mods.speed);
    } else {
      DLOG("Pokemon #%d: %s \n", i, get_pokemon_name(p.id));
    }
    DLOG("HP: %d \t", p.hp);
    DLOG("Level: %d \t", p.stats.level);
    DLOG("Types: %d, %d \t", p.type1, p.type2);
    DLOG("Available Moves: \t");
    for (int j = 0; j < 4; j++) {
      Move m = p.poke_moves[j];
      DLOG(
          "\t Label: %s; PP Remaining: %d; Power: %d; Accuracy: %.2f; Type: "
          "%d; Category: %d \n",
          get_move_name(m.id),
          m.pp,
          m.power,
          m.accuracy,
          m.type,
          m.category);
    }
  }
}



int make_move(Battle* b, Player* user, Player* target, int input, int type) {}


// Takes inputs move from both players.
// Resolves queue until choice is required from one (or both) players
// Ignores choices that aren't valid. 
void step(Battle* b, int p1_choice, int p2_choice) {
  print_state(b->p1);
  printf("\n\n");
  print_state(b->p2);
  // Sort & evaluate the battlequeue on a move by move basis
  eval_queue(b);
}
