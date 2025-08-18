#ifndef LOG_H
#define LOG_H

#if DEBUG
  #include <stdio.h>
  #include "move_labels.h"
  #include "pokedex_labels.h"

  inline char* get_move_name(int move_id) {
      return MOVE_LABELS[move_id];
    }
    inline char* get_pokemon_name(int pokemon_id) {
      return POKEDEX_LABELS[pokemon_id];
    }
  #define DLOG(fmt, ...)                                                       \
    do {                                                                       \
      fprintf(stderr, "[%s:%d] " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__); \
    } while (0)

    #else

inline char* get_move_name(int move_id) {
  return 0;
}

inline char* get_pokemon_name(int pokemon_id) {
  return 0;
}

#define DLOG(fmt, ...) \
  do {                 \
  } while (0)  // No-op in release builds


#endif
#endif  // LOG_H