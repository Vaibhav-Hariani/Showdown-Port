#include "pokegen.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "pokedex_labels.h"
#include "move_labels.h"

int main() {
  srand(time(NULL));
  int random_pokemon = rand() % NUM_POKEMON;
  const MOVE_IDS* learnset = LEARNSETS[random_pokemon];
  int learnset_len = LEARNSET_LENGTHS[random_pokemon];

  printf("Selected Pokemon: %d %s\n", random_pokemon, PokemonNames[random_pokemon]);
  printf("Learnset: ");
  for (int i = 0; i < learnset_len; i++) {
    printf("%d %s, ", learnset[i], MoveLabels[learnset[i]]);
  }
  printf("\n");

  MOVE_IDS selected_moves[4] = {NO_MOVE, NO_MOVE, NO_MOVE, NO_MOVE};
  generate_moveset(selected_moves, learnset, learnset_len);

  printf("Generated Pokemon: %d %s\n", random_pokemon, PokemonNames[random_pokemon]);
  for (int i = 0; i < 4; i++) {
    printf("Move %d: %s\n", i + 1, MoveLabels[selected_moves[i]]);
  }
  return 0;
}
