#ifndef MOVE_STRUCTS_H
#define MOVE_STRUCTS_H

#include "generated_move_enum.h"
#include "typing.h"

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
  int power;
  float accuracy;
  TYPE type;
  MOVE_CATEGORY category;
  int pp;
  void (*movePtr)(Battle*, BattlePokemon*, BattlePokemon*);
  int priority;
};

#endif
