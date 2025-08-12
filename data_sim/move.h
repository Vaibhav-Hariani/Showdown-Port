#ifndef MOVE_H
#define MOVE_H

#include <stddef.h> // For NULL definition
#include "pokemon.h"
#include "move_enum.h"
#include "typing.h"

struct STR_MOVES {
  move_ids id;
  // Moves are currently being defined as functions.
  // each move obj contains a pointer to it'
  //  is a pointer to said function.
  int (*movePtr)(battle* , pokemon *, pokemon *, int);
  int power;
  int accuracy;
  TYPE move_type;
  int pp;
} typedef move;

// Define an array of moves indexed by move_enum.h
const move moves[] = {
    [TACKLE] = {
        .id = TACKLE,
        .movePtr = NULL, // Placeholder for now
        .power = 40,
        .accuracy = 100,
        .move_type = NORMAL,
        .pp = 35
    },
    // Add more moves here
};


#endif