#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "generated_learnsets.h"
#include "move_labels.h"
#include "pokedex_labels.h"

const int NUM_POKEMON = 151;

void generate_moveset(MOVE_IDS out_moves[4],
                      const MOVE_IDS* learnset,
                      int learnset_len) {
  // choose 4 random moves from a learnset
  // and store it in out_moves
  int selected[4] = {NO_MOVE, NO_MOVE, NO_MOVE, NO_MOVE};
  int num_selected = 0;
  while (num_selected < 4 && num_selected < learnset_len) {
    int random_index = rand() % learnset_len;
    MOVE_IDS move = learnset[random_index];
    // check if move is already selected
    int already_selected = 0;
    for (int i = 0; i < num_selected; i++) {
      if (selected[i] == move) {
        already_selected = 1;
        break;
      }
    }
    if (!already_selected && move != NO_MOVE) {
      selected[num_selected] = move;
      num_selected++;
    }
  }
  for (int i = 0; i < 4; i++) {
    out_moves[i] = selected[i];
  }
}

int main() {
  // create a random pokemon
  srand(time(NULL));

  int random_pokemon = rand() % NUM_POKEMON;
  // get the learnset for that pokemon
  const MOVE_IDS* learnset = LEARNSETS[random_pokemon];
  int learnset_len = LEARNSET_LENGTHS[random_pokemon];

  printf("Selected Pokemon: %d %s\n",
         random_pokemon,
         PokemonNames[random_pokemon]);
  // print learnset
  printf("Learnset: ");
  for (int i = 0; i < learnset_len; i++) {
    printf("%d %s, ", learnset[i], MoveLabels[learnset[i]]);
  }
  printf("\n");

  // grab 4 unique moves from the learnset
  MOVE_IDS selected_moves[4] = {NO_MOVE, NO_MOVE, NO_MOVE, NO_MOVE};
  generate_moveset(selected_moves, learnset, learnset_len);

  // print the pokemon and its moves
  printf("Generated Pokemon: %d %s\n",
         random_pokemon,
         PokemonNames[random_pokemon]);
  for (int i = 0; i < 4; i++) {
    printf("Move %d: %s\n", i + 1, MoveLabels[selected_moves[i]]);
  }
}