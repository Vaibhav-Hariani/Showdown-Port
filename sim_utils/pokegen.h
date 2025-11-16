#ifndef POKEGEN_H
#define POKEGEN_H

#include "../data_sim/generated_learnsets.h"
// Needed for poke_ref and pokemon_base
#include "../data_sim/generated_movedex.h"
#include "../data_sim/poke_enum.h"
#include "../data_sim/pokedex.h"
#include "move_structs.h"
#include "poke_structs.h"
#include "string.h"

#define NUM_POKEMON ((int)LAST_POKEMON)

// DV * 15 + floor(statexp/4)
#define MAX_CONST_STATBOOST 93

void generate_moveset(MOVE_IDS out_moves[4],
                      const MOVE_IDS* learnset,
                      int learnset_len) {
  // choose 4 random moves from a learnset
  // and store it in out_moves
  MOVE_IDS selected[4] = {NO_MOVE, NO_MOVE, NO_MOVE, NO_MOVE};
  int num_selected = 0;
  while (num_selected < 4 && num_selected < learnset_len) {
    int random_index = rand() % learnset_len;
    MOVE_IDS move = learnset[random_index];
    // check if move is already selected
    MOVE_IDS already_selected = 0;
    for (int i = 0; i < num_selected; i++) {
      if (selected[i] == move) {
        already_selected = 1;
        break;
      }
    }
    if (!already_selected && move != NO_MOVE) {
      selected[num_selected] = move;
      num_selected++;
    }
  }
  for (int i = 0; i < 4; i++) {
    out_moves[i] = selected[i];
  }
}
// take an input pokemon object, and load it with moves, DV's/EV's (optionally)
// Can expand to take in more data
void load_pokemon(Pokemon* ret, MOVE_IDS* move_ids, POKEDEX_IDS poke_id) {
  // int* evs,
  // int* ivs,
  // int* opt_level
  // 1. Lookup base stats/types

  while (poke_id == MISSINGNO) {
    poke_id = rand() % NUM_POKEMON;
  }
  const poke_ref* base = &POKEMON_BASE[poke_id];
  ret->id = poke_id;
  ret->type1 = base->primary_type;
  ret->type2 = base->secondary_type;

  // 2. Stats: base stats + IVs/EVs
  // int statexp = 63;  // Gen 1 max is 65535, floor(sqrt / 4) is 63
  // int dv = 15;  // Gen 1 max
  // int level = 100;
  ret->stats.level = 100;

  // 4. HP
  // Assuming max level simplifies this transaction dramatically
  int hp = base->base_stats[STAT_HP] * 2 + MAX_CONST_STATBOOST + 110;
  ret->stats.base_stats[STAT_HP] = hp;
  ret->max_hp = hp;
  ret->hp = hp;
  for (int i = 1; i < STAT_COUNT; i++) {
    // maxxing out for gen1
    // Gen1 had StatExp instead of evs:
    // https://bulbapedia.bulbagarden.net/wiki/Stat#Formula Can be maxxed right
    // away.
    ret->stats.base_stats[i] =
        base->base_stats[i] * 2 + MAX_CONST_STATBOOST + 5;
  }
  // 3. Moves
  if (move_ids) {
    for (int i = 0; i < 4; i++) {
      MOVE_IDS id = move_ids[i];
      ret->poke_moves[i] = MOVES[id];
    }
  } else {
    // Generate moveset from learnset
    const MOVE_IDS* learnset = LEARNSETS[poke_id];
    int learnset_len = LEARNSET_LENGTHS[poke_id];
    MOVE_IDS moves[4] = {NO_MOVE, NO_MOVE, NO_MOVE, NO_MOVE};
    generate_moveset(moves, learnset, learnset_len);
    for (int i = 0; i < 4; i++) {
      MOVE_IDS id = moves[i];
      ret->poke_moves[i] = MOVES[id];
    }
  }
  // 3a. Struggle copy
  memcpy(&ret->struggle, MOVES+STRUGGLE_MOVE_ID, sizeof(Move));

  memset(&ret->status, 0, sizeof(ret->status));
}
#endif
