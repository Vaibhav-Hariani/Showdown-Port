#ifndef BATTLE_STRUCTS_H
#define BATTLE_STRUCTS_H

#include "poke_structs.h"
#include "queue_structs.h"
typedef struct STR_PLAYER {
  Pokemon team[6];
  BattlePokemon active_pokemon;
  char active_pokemon_index;
  // Bitfield: 6 bits, tracks which opponent Pokémon have been shown
  unsigned int shown_pokemon : 6;
} Player;

typedef struct STR_BATTLE {
  Player p1;
  Player p2;
  battlequeue action_queue;
  uint16_t turn_num;    // Changed from int to uint16_t (max turns ~65k)
  Move* lastMove;
  int16_t lastDamage;   // Changed from int to int16_t (max damage ~500)
  // Used to determine if a pokemon needs switching
  int mode;
} Battle;
#endif