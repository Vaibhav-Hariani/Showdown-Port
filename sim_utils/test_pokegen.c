#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../data_labels/move_labels.h"
#include "../data_labels/pokedex_labels.h"
#include "../data_sim/poke_enum.h"
#include "../data_sim/pokedex.h"
#include "pokegen.h"

int stat_test(POKEDEX_IDS poke) {
  const MOVE_IDS* learnset = LEARNSETS[poke];
  int learnset_len = LEARNSET_LENGTHS[poke];
  Pokemon p = {0};
  printf("Selected Pokemon: %d %s\n", poke, POKE_NAMES[poke]);
  load_pokemon(&p, NULL, &poke);

  MOVE_IDS selected_moves[4] = {NO_MOVE, NO_MOVE, NO_MOVE, NO_MOVE};
  generate_moveset(selected_moves, learnset, learnset_len);

  printf("Generated Pokemon: %d %s\n", poke, POKE_NAMES[poke]);
  // Print computed stats (HP shown as current/max)
  printf("Stats - HP: %d/%d, Atk: %d, Def: %d, SpA: %d, SpD: %d, Spe: %d\n",
         p.hp,
         p.max_hp,
         p.stats.base_stats[STAT_ATTACK],
         p.stats.base_stats[STAT_DEFENSE],
         p.stats.base_stats[STAT_SPECIAL_ATTACK],
         p.stats.base_stats[STAT_SPECIAL_DEFENSE],
         p.stats.base_stats[STAT_SPEED]);
  for (int i = 0; i < 4; i++) {
    printf("Move %d: %s\n", i + 1, MOVE_LABELS[selected_moves[i]]);
  }

  return 0;
}
int main() {
  // srand(time(NULL));
  POKEDEX_IDS random_pokemon = CHARIZARD;
  return stat_test(random_pokemon);
}