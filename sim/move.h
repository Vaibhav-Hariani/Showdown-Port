#ifndef MOVE_H
#define MOVE_H

#include "pokemon.h"
#include "move_enum.h"

struct STR_MOVES {
  move_ids id;
  // Moves are currently being defined as functions.
  // each move obj contains a pointer to it'
  //  is a pointer to said function.
  int (*movePtr)(battle* , pokemon *, pokemon *, int);
  int pp;
} typedef move;

#endif