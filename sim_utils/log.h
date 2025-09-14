#ifndef LOG_H
#define LOG_H

#if DEBUG
#include <stdio.h>

#include "../data_labels/move_labels.h"
#include "../data_labels/pokedex_labels.h"

char* get_move_name(int move_id) { return MoveLabels[move_id]; }
char* get_pokemon_name(int pokemon_id) { return PokemonNames[pokemon_id]; }
#define DLOG(fmt, ...)                                                       \
  do {                                                                       \
    fprintf(stderr, "[%s:%d] " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__); \
  } while (0)

#else

char* get_move_name(int move_id) { return 0; }

char* get_pokemon_name(int pokemon_id) { return 0; }

#define DLOG(fmt, ...) \
  do {                 \
  } while (0)  // No-op in release builds

#endif
#endif  // LOG_H