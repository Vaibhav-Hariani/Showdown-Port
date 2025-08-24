#ifndef POKEMON_BASE_H
#define POKEMON_BASE_H

#include "poke_enum.h"
#include "typing.h"

//Used to define base pokemon data, pre-IVS/EVs/DVs/Modifiers

typedef struct {
    POKEDEX_IDS id;
    int base_stats[STAT_COUNT]; // Include base stats directly
    TYPE primary_type;
    TYPE secondary_type;
} poke_ref;

static const poke_ref pokemon_base[] = {
    {BULBASAUR, {45, 49, 49, 65, 65, 45}, GRASS, POISON},
    {IVYSAUR, {60, 62, 63, 80, 80, 60}, GRASS, POISON},
    {VENUSAUR, {80, 82, 83, 100, 100, 80}, GRASS, POISON},
    // Add more Pokémon here
};
#endif