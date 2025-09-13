#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "log.h"
// Needed for poke_ref and pokemon_base
#include "pokegen.h"

int main() {
  // create a random pokemon
  srand(time(NULL));

  int random_pokemon = rand() % NUM_POKEMON;
  // get the learnset for that pokemon
  const MOVE_IDS* learnset = LEARNSETS[random_pokemon];
  int learnset_len = LEARNSET_LENGTHS[random_pokemon];
  DLOG("Selected Pokemon: %d %s\n",
         random_pokemon,
         get_pokemon_name(random_pokemon));
  // print learnset
  DLOG("Learnset: ");
  for (int i = 0; i < learnset_len; i++) {
    DLOG("%d %s, ", learnset[i], get_move_name(learnset[i]));
  }
  DLOG("\n");

  // grab 4 unique moves from the learnset
  MOVE_IDS selected_moves[4] = {NO_MOVE, NO_MOVE, NO_MOVE, NO_MOVE};
  generate_moveset(selected_moves, learnset, learnset_len);

  // print the pokemon and its moves
  DLOG("Generated Pokemon: %d %s\n",
         random_pokemon,
         get_pokemon_name(random_pokemon));
  for (int i = 0; i < 4; i++) {
    DLOG("Move %d: %s\n", i + 1, get_move_name(selected_moves[i]));
  }
}