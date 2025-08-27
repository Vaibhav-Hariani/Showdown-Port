#ifndef LOG_H
#define LOG_H
#include <stdio.h>

#include "generated_move_enum.h"
#include "move_labels.h"
#include "pokedex_labels.h"

const char* get_move_name(MOVE_IDS move_id);
const char* get_pokemon_name(int pokemon_id);

#if DEBUG

#define DLOG(fmt, ...)                                                       \
  do {                                                                       \
    fprintf(stderr, "[%s:%d] " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__); \
  } while (0)

#else

#define DLOG(fmt, ...) \
  do {                 \
  } while (0)  // No-op in release builds

#endif
#endif  // LOG_H