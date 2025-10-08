#ifndef SIM_PACKING_H
#define SIM_PACKING_H

#include "sim_utils/battle.h"
#include "sim_utils/move.h"
#include "stdint.h"

typedef struct {
  int16_t p1_choice; // raw encoded choice (switch idx or move slot)
  int16_t p1_val;    // switch idx or resolved move id
  int16_t p2_choice;
  int16_t p2_val;    // switch idx or resolved move id
} PrevChoices;

// Constants
#define PACK_HEADER_INTS 8
#define PACK_POKE_INTS 7
#define PACK_TEAM_SLOTS 6
#define PACK_TOTAL_POKEMON (PACK_TEAM_SLOTS * 2)
#define PACK_TOTAL_INTS (PACK_HEADER_INTS + PACK_TOTAL_POKEMON * PACK_POKE_INTS)

// Packing function declarations
static inline int16_t pack_attack_def_specA_specD(stat_mods* mods);
static inline int16_t pack_stat_acc_eva(stat_mods* mods);
static inline int16_t pack_move(Move* move, int disabled);
static inline int16_t pack_status_and_volatiles(Player* player, int poke_index);
static inline int16_t pack_hp_percent(Pokemon* p);
// player_index: 1 for p1 (observer perspective), 2 for p2; used to gate hidden info
static inline void pack_poke(int16_t* row,
                             Player* player,
                             int player_index,
                             int poke_index,
                             int is_active,
                             int hidden);
void pack_battle(Battle* b, int16_t* out, PrevChoices* prev);

// ============================================================================
// Implementation
// ============================================================================
static inline int16_t pack_attack_def_specA_specD(stat_mods* mods) {
  int16_t packed = 0;
  packed |= (mods->attack & 0xF) << 0;
  packed |= (mods->defense & 0xF) << 4;
  packed |= (mods->specA & 0xF) << 8;
  packed |= (mods->specD & 0xF) << 12;
  return packed;
}

static inline int16_t pack_stat_acc_eva(stat_mods* mods) {
  int16_t packed = 0;
  packed |= (mods->speed & 0xF) << 0;
  packed |= (mods->accuracy & 0xF) << 4;
  packed |= (mods->evasion & 0xF) << 8;
  return packed;
}

// Packs move data into int16_t per spec (see header comment)
static inline int16_t pack_move(Move* move, int disabled) {
  if (!move) {
    return 0; // unseen / null
  }
  int16_t packed = 0;
  packed |= (int16_t)(move->id & 0xFF);          // bits 0-7 move id
  int pp = move->pp;
  if (pp < 0) pp = 0; if (pp > 31) pp = 31;
  packed |= (int16_t)(pp & 0x1F) << 8;           // bits 8-12 PP
  packed |= (int16_t)((disabled ? 1 : 0) & 0x1) << 13; // bit13 disabled
  return packed;
}

// HP percent *1000 in bits 0-9; bits 10-15 reserved (currently 0)
static inline int16_t pack_hp_percent(Pokemon* p) {
  if (!p || p->max_hp <= 0) return 0;
  int hp = p->hp;
  if (hp < 0) hp = 0;
  if (hp > p->max_hp) hp = p->max_hp;
  int scaled = (hp * 1000) / p->max_hp; // 0..1000
  if (scaled > 1000) scaled = 1000;
  int16_t packed = (int16_t)(scaled & 0x3FF); // lower 10 bits
  return packed; // upper 6 bits zero for now
}

// Status & volatile bits; limited by what current engine tracks.
static inline int16_t pack_status_and_volatiles(Player* player, int poke_index) {
  Pokemon* p = &player->team[poke_index];
  int16_t packed = 0;
  // Core status
  packed |= (p->status.paralyzed & 0x1) << 0; // PAR
  packed |= (p->status.burn & 0x1) << 1;      // BRN
  packed |= (p->status.freeze & 0x1) << 2;    // FRZ
  packed |= (p->status.poison & 0x1) << 3;    // PSN (regular)
  packed |= ((p->status.sleep > 0) ? 1 : 0) << 4; // SLP
  // Toxic (badly poisoned) tracked via active battle pokemon counter
  if (poke_index == player->active_pokemon_index) {
    if (player->active_pokemon.badly_poisoned_ctr > 0) {
      packed |= 1 << 5; // TOX
    }
    if (player->active_pokemon.confusion_counter > 0) {
      packed |= 1 << 6; // CONFUSED
    }
    if (player->active_pokemon.flinch) {
      packed |= 1 << 14; // FLINCH
    }
  }
  // Bits for SEEDED / SUB / RAGE / DISABLED / PARTIALLY_TRAPPED / TRANSFORMED / FOCUS_ENERGY
  // Not currently implemented in structs; remain 0 as placeholders.
  return packed;
}

static inline void pack_poke(int16_t* row,
                             Player* player,
                             int player_index,
                             int poke_index,
                             int is_active,
                             int hidden) {
  if (hidden) {
    for (int i = 0; i < PACK_POKE_INTS; i++) row[i] = 0;
    return;
  }

  Pokemon* poke = &player->team[poke_index];
  // species id (negated if active)
  int16_t species = (int16_t)poke->id;
  if (is_active) species = (int16_t)(-species);
  row[0] = species;

  for (int k = 0; k < 4; k++) {
    Move* m = &poke->poke_moves[k];
    int reveal = (player_index == 1) ? 1 : (m->revealed ? 1 : 0);
    row[1 + k] = reveal ? pack_move(m, 0) : 0;
  }
  row[5] = pack_hp_percent(poke);
  row[6] = pack_status_and_volatiles(player, poke_index);
}

void pack_battle(Battle* b, int16_t* out, PrevChoices* prev) {
  // Header (choices + active stat mods only)
  out[0] = prev->p1_choice;
  out[1] = prev->p1_val;
  out[2] = prev->p2_choice;
  out[3] = prev->p2_val;

  // Reveal moves used on previous turn (choice 0-3) by setting move.revealed
  if (prev->p1_choice >= 0 && prev->p1_choice < 4 && b->p1.active_pokemon_index >= 0) {
    b->p1.team[b->p1.active_pokemon_index].poke_moves[prev->p1_choice].revealed = 1;
  }
  if (prev->p2_choice >= 0 && prev->p2_choice < 4 && b->p2.active_pokemon_index >= 0) {
    b->p2.team[b->p2.active_pokemon_index].poke_moves[prev->p2_choice].revealed = 1;
  }
  stat_mods* p1mods = &b->p1.active_pokemon.stat_mods;
  stat_mods* p2mods = &b->p2.active_pokemon.stat_mods;
  out[4] = pack_attack_def_specA_specD(p1mods);
  out[5] = pack_stat_acc_eva(p1mods);
  out[6] = pack_attack_def_specA_specD(p2mods);
  out[7] = pack_stat_acc_eva(p2mods);

  // Pokémon rows (interleaved)
  for (int slot = 0; slot < PACK_TEAM_SLOTS; slot++) {
    int interleave_index_p1 = slot * 2 + 0;
    int base_offset_p1 = PACK_HEADER_INTS + interleave_index_p1 * PACK_POKE_INTS;
  pack_poke(out + base_offset_p1,
        &b->p1,
        1,
        slot,
        (slot == b->p1.active_pokemon_index),
        0);

    int hidden = 0;
    if (!(b->p1.shown_pokemon & (1u << slot))) hidden = 1;
    int interleave_index_p2 = slot * 2 + 1;
    int base_offset_p2 = PACK_HEADER_INTS + interleave_index_p2 * PACK_POKE_INTS;
    pack_poke(out + base_offset_p2,
              &b->p2,
              2,
              slot,
              (slot == b->p2.active_pokemon_index),
              hidden);
  }
}

#endif