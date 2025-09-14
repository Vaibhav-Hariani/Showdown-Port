#ifndef BATTLE_STRUCTS_H
#define BATTLE_STRUCTS_H
#include "poke_structs.h"
#include "queue_structs.h"
typedef struct STR_PLAYER {
  Pokemon* team;  // Changed to pointer to allow assignment from global array
  BattlePokemon active_pokemon;
  char active_pokemon_index;
} Player;

typedef struct STR_BATTLE {
  Player p1;
  Player p2;
  battlequeue action_queue;
  int turn_num;
  Move* lastMove;
  int lastDamage;
  //Used to determine if a pokemon needs switching
  int mode;
} Battle;
#endif