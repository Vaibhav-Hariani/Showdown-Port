#ifndef SIM_PACKING_H
#define SIM_PACKING_H

#include "sim_utils/battle.h"
#include "sim_utils/move.h"
#include "stdint.h"

// Packing function declarations
int16_t pack_attack_def_specA_specD(stat_mods* mods);
int16_t pack_stat_acc_eva(stat_mods* mods);
int16_t pack_move(Move* move);
int16_t pack_status(Pokemon* p);
void pack_poke(int16_t* row, Player* player, int poke_index);
void pack_battle(Battle* b, int16_t* out);

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

void pack_battle(Battle* b, int16_t* out) {
  // Each pokemon: [id, move1, move2, move3, move4, hp, status_flags, stat_mod1,
  // stat_mod2] NUM_POKE pokemon per player, 2 players Active pokemon have 2 extra ints
  // for stat mods Flattened array: (NUM_POKE * 2) rows * 9 columns = (NUM_POKE * 18) elements total
  for (int i = 0; i < 2; i++) {
    Player* p = get_player(b, i + 1);
    for (int j = 0; j < NUM_POKE; j++) {
      int pokemon_index = i * NUM_POKE + j;
      int base_offset = pokemon_index * 9;
      int16_t* row = out + base_offset;
      // Forcing the obscuring of unseen opponent pokemon
      if (i == 2 && !(b->p1.shown_pokemon & (1 << j))) {
        for (int z = 0; z < 9; z++) row[z] = 0;
        continue;
      }
      pack_poke(row, p, j);
    }
  }
}

#endif