#ifndef MOVE_STRUCTS_H
#define MOVE_STRUCTS_H

#include "../data_sim/generated_move_enum.h"
#include "../data_sim/typing.h"

// Typedefs for convenience
typedef struct STR_BATTLE Battle;
typedef struct STR_PLAYER Player;
typedef struct STR_ACTION Action;
typedef struct STR_BQUEUE battlequeue;
typedef struct STR_MOVE Move;

// Relevant forward declarations
typedef struct STR_POKE Pokemon;
typedef struct STR_BATTLE_POKEMON BattlePokemon;

// Complete struct, enum, and union definitions for moves

struct STR_MOVE {
  MOVE_IDS id;
  uint8_t power;      // Changed from int to uint8_t (max move power ~250)
  uint8_t accuracy;   // Changed from float to uint8_t (0-255, where 255 = 100%)
  TYPE type;
  MOVE_CATEGORY category;
  uint8_t pp;         // Changed from int to uint8_t (max PP is 64)
  void (*movePtr)(Battle*, BattlePokemon*, BattlePokemon*);
  int8_t priority;    // Changed from int to int8_t (range -7 to +5)
  // Flag: set to 1 once the move has been revealed/used (for opponent
  // visibility)
  unsigned char revealed;
};

#endif
