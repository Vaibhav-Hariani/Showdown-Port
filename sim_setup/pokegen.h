#ifndef POKEGEN_H
#define POKEGEN_H

#include "generated_learnsets.h"
// Needed for poke_ref and pokemon_base
#include "../data_sim/generated_movedex.h"
#include "move_structs.h"
#include "poke_enum.h"
#include "poke_structs.h"
#include "pokedex.h"

#define NUM_POKEMON ((int)LAST_POKEMON)

void generate_moveset(MOVE_IDS out_moves[4],
                      const MOVE_IDS* learnset,
                      int learnset_len) {
  // choose 4 random moves from a learnset
  // and store it in out_moves
  int selected[4] = {NO_MOVE, NO_MOVE, NO_MOVE, NO_MOVE};
  int num_selected = 0;
  while (num_selected < 4 && num_selected < learnset_len) {
    int random_index = rand() % learnset_len;
    MOVE_IDS move = learnset[random_index];
    // check if move is already selected
    int already_selected = 0;
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
void load_pokemon(Pokemon* ret,
                  int* opt_id,
                  int* evs,
                  int* ivs,
                  int* move_ids,
                  int* opt_level) {
  // 1. Lookup base stats/types
  int pokedex_id = opt_id ? *opt_id : (rand() % NUM_POKEMON);
  const poke_ref* base = &pokemon_base[pokedex_id];
  ret->id = pokedex_id;
  ret->type1 = base->primary_type;
  ret->type2 = base->secondary_type;

  // 2. Stats: base stats + IVs/EVs
  int max_ev = 65535;  // Gen 1 max
  int max_iv = 15;     // Gen 1 max
  int level = opt_level ? *opt_level : 100;
  ret->stats.level = level;
  for (int i = 0; i < STAT_COUNT; i++) {
    int base_stat = base->base_stats[i];
    ret->stats.base_stats[i] = base_stat;
    // TODO: Figure out how EVs n IVs work in gen1 and implement here
    //  int ev = evs ? evs[i] : max_ev;
    //  int iv = ivs ? ivs[i] : max_iv;
    //  if (i == STAT_HP) {
    //    ret->stats.base_stats[i] =
    //        (((base_stat + iv) * 2 + (ev / 4)) * level / 100) + level + 10;
    //  } else {
    //    ret->stats.base_stats[i] =
    //        (((base_stat + iv) * 2 + (ev / 4)) * level / 100) + 5;
  }
  // 3. Moves
  if (move_ids) {
    for (int i = 0; i < 4; i++) {
      MOVE_IDS id = move_ids[i];
      ret->poke_moves[i] = MOVES[id];
    }
  } else {
    // Generate moveset from learnset
    const MOVE_IDS* learnset = LEARNSETS[pokedex_id];
    int learnset_len = LEARNSET_LENGTHS[pokedex_id];
    MOVE_IDS moves[4] = {NO_MOVE, NO_MOVE, NO_MOVE, NO_MOVE};
    generate_moveset(moves, learnset, learnset_len);
    for (int i = 0; i < 4; i++) {
      MOVE_IDS id = moves[i];
      ret->poke_moves[i] = MOVES[id];
    }
  }
  // 4. HP
  ret->max_hp = ret->stats.base_stats[STAT_HP];
  ret->hp = ret->max_hp;
  // 5. Status
  // Zero out status flags
  memset(&ret->status, 0, sizeof(ret->status));
}
#endif