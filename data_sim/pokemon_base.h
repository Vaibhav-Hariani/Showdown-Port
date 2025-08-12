#ifndef POKEMON_BASE_H
#define POKEMON_BASE_H

#include "pokedex.h"
#include "typing.h"
//Used to define base pokemon data, pre-IVS/EVs/DVs/Modifiers

typedef struct {
  POKEDEX id;
  // Base stats: HP, Attack, Defense, Special Attack, Special Defense, Speed
  int base_stats[6];
  TYPE primary_type;
  TYPE secondary_type;
} poke_ref;

static const poke_ref pokemon_base[] = {
  {BULBASAUR, {45, 49, 49, 65, 65}, GRASS, POISON},
  {IVYSAUR, {60, 62, 63, 80, 80}, GRASS, POISON},
  {VENUSAUR, {80, 82, 83, 100, 100}, GRASS, POISON},
  // Add more Pokémon here
};
#endif