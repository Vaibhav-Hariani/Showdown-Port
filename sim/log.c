#include "log.h"

const char* get_move_name(MOVE_IDS move_id) { return MoveLabels[move_id]; }
const char* get_pokemon_name(int pokemon_id) {
  return PokemonNames[pokemon_id];
}
