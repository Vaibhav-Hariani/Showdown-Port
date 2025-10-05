#ifndef SIM_PACKING_H
#define SIM_PACKING_H

#include "sim_utils/battle.h"
#include "sim_utils/move.h"
#include "stdint.h"

// Previous choices for both players
typedef struct {
  int16_t p1_choice; // raw encoded choice (switch idx or move slot)
  int16_t p1_val;    // switch idx or resolved move id
  int16_t p2_choice;
  int16_t p2_val;    // switch idx or resolved move id
} PrevChoices;

// Packing function declarations
int16_t pack_attack_def_specA_specD(stat_mods* mods);
int16_t pack_stat_acc_eva(stat_mods* mods);
int16_t pack_move(Move* move);
int16_t pack_status(Pokemon* p);
void pack_poke(int16_t* row, Player* player, int poke_index);
// Observation layout: [ action_row (9 ints) , (NUM_POKE*2) * 9 pokemon ints ]
// action_row: p1_choice, p1_val, p2_choice, p2_val, pad x5
void pack_battle(Battle* b, int16_t* out, PrevChoices* prev);

// Implementation
int16_t pack_attack_def_specA_specD(stat_mods* mods) {
  int16_t packed = 0;
  packed |= (mods->attack & 0xF) << 0;
  packed |= (mods->defense & 0xF) << 4;
  packed |= (mods->specA & 0xF) << 8;
  packed |= (mods->specD & 0xF) << 12;
  return packed;
}

int16_t pack_stat_acc_eva(stat_mods* mods) {
  int16_t packed = 0;
  packed |= (mods->speed & 0xF) << 0;
  packed |= (mods->accuracy & 0xF) << 4;
  packed |= (mods->evasion & 0xF) << 8;
  return packed;
}

// Packs move data for a single move into an int16_t
// Format: [move_id(8 bits), pp(6 bits)] - 2 bits unused
int16_t pack_move(Move* move) {
  int16_t packed = 0;
  packed |= (int16_t)(move->id & 0xFF) << 0;  // 8 bits for move ID (0-255)
  packed |= (int16_t)(move->pp & 0x3F) << 8;  // 6 bits for PP (0-63)
  return packed;
}

// Packs all pokemon in the battle into the provided int array.
// Each pokemon: [id, hp, status_flags, (stat_mods if active), move1, move2,
// move3, move4] Returns the number of ints written.
int16_t pack_status(Pokemon* p) {
  int16_t packed = 0;
  packed |= (p->status.paralyzed & 0x1) << 0;
  packed |= (p->status.burn & 0x1) << 1;
  packed |= (p->status.freeze & 0x1) << 2;
  packed |= (p->status.poison & 0x1) << 3;
  packed |= (p->status.sleep & 0x1) << 4;

  return packed;
}

void pack_poke(int16_t* row, Player* player, int poke_index) {
  Pokemon* poke = &player->team[poke_index];

  // Pack pokemon number first
  row[0] = poke->id;

  // Pack move data next (positions 1-4)
  for (int k = 0; k < 4; k++) {
    row[1 + k] = pack_move(&poke->poke_moves[k]);
  }

  // Pack everything else after move data
  row[5] = poke->hp;
  // Also contains confusion if the pokemon is active (and confused)
  row[6] = pack_status(poke);

  if (poke_index == player->active_pokemon_index) {
    row[0] *= -1;  // Mark active pokemon with negative id
    stat_mods* mods = &player->active_pokemon.stat_mods;
    row[7] = pack_attack_def_specA_specD(mods);
    row[8] = pack_stat_acc_eva(mods);
  } else {
    row[7] = 0;
    row[8] = 0;
  }
}

void pack_battle(Battle* b, int16_t* out, PrevChoices* prev) {
  out[0] = prev->p1_choice;
  out[1] = prev->p1_val;
  out[2] = prev->p2_choice;
  out[3] = prev->p2_val;
  for (int pad = 4; pad < 9; pad++) out[pad] = 0;

  // Each pokemon row: [id, move1, move2, move3, move4, hp, status_flags, stat_mod1, stat_mod2]
  // Interleaved ordering for scalability: p1_poke1, p2_poke1, p1_poke2, p2_poke2, ...
  // Flattened after action row: (NUM_POKE * 2) rows * 9 columns.
  for (int j = 0; j < NUM_POKE; j++) {
    for (int i = 0; i < 2; i++) {
      Player* p = get_player(b, i + 1); // i=0 -> p1, i=1 -> p2
      int interleave_index = j * 2 + i; // interleaved slot
      int base_offset = 9 + interleave_index * 9; // +9 to skip action row
      int16_t* row = out + base_offset;
      // Obscure unseen opponent pokemon (only applies when looking at opponent's roster)
      // Player 1's view: hide p2 mons not yet shown; Player 2's view: hide p1 mons not yet shown.
      // Since we produce a symmetric observation (no per-player perspective), we conservatively
      // hide only for truly unseen opponent mons. Here we assume p1.shown_pokemon tracks p2 reveals
      // and p2.shown_pokemon tracks p1 reveals.

      if (i == 1) { // p2 pokemon potentially hidden from p1
        if (!(b->p1.shown_pokemon & (1u << j))) {
          continue;
        }
      }
      pack_poke(row, p, j);
    }
  }
}

#endif