#ifndef SIM_PACKING_H
#define SIM_PACKING_H

#include <stdint.h>

#include "sim_utils/battle.h"
#include "sim_utils/move.h"

typedef struct {
  int16_t p1_choice;  // raw encoded choice (switch idx or move slot)
  int16_t p1_val;     // switch idx or resolved move id
  int16_t p2_choice;
  int16_t p2_val;  // switch idx or resolved move id
} PrevChoices;

#define PACK_HEADER_INTS 4
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
// player_index: 1 for p1 (observer perspective), 2 for p2; used to gate hidden
// info
static inline void pack_poke(int16_t* row,
                             Player* player,
                             int player_index,
                             int poke_index,
                             int is_active,
                             int hidden);

// Pack observation for a specific player's perspective
void pack_battle(Battle* b, Player* observer, Player* opponent, int16_t* out);

// Pack observations for all agents (up to 2 agents)
void pack_all_agents(Battle* b, int num_agents, int16_t* out);

// ============================================================================
// Implementation
// ============================================================================
static inline int16_t pack_attack_def_specA_specD(stat_mods* mods) {
  int16_t packed = 0;
  packed |= ((mods->attack + 6) & 0xF) << 0;
  packed |= ((mods->defense + 6) & 0xF) << 4;
  packed |= ((mods->specA + 6) & 0xF) << 8;
  packed |= ((mods->specD + 6) & 0xF) << 12;
  return packed;
}

static inline int16_t pack_stat_acc_eva(stat_mods* mods) {
  int16_t packed = 0;
  packed |= ((mods->speed + 6) & 0xF) << 0;
  packed |= ((mods->accuracy + 6) & 0xF) << 4;
  packed |= ((mods->evasion + 6) & 0xF) << 8;
  return packed;
}

// Packs move data into int16_t per spec (see header comment)
static inline int16_t pack_move(Move* move, int disabled) {
  if (!move) {
    return 0;  // unseen / null
  }
  int16_t packed = 0;
  int move_id = move->id & 0xFF;
  int pp = move->pp;
  // If the move has zero or negative PP (and isn't Struggle), report Struggle
  if ((pp <= 0) && (move_id != STRUGGLE_MOVE_ID)) {
    move_id = STRUGGLE_MOVE_ID;
    pp = 0;
  }
  // Clamp PP to 0..63 for packing
  if (pp < 0) pp = 0;
  if (pp > 63) pp = 63;
  packed |= (int16_t)(move_id & 0xFF);                  // bits 0-7 move id
  packed |= (int16_t)(pp & 0x3F) << 8;                  // bits 8-13 PP (6 bits)
  packed |= (int16_t)((disabled ? 1 : 0) & 0x1) << 14;  // bit14 disabled
  return packed;
}

// HP percent *1000 in bits 0-9; bits 10-15 reserved (currently 0)
static inline int16_t pack_hp_percent(Pokemon* p) {
  if (!p || p->max_hp <= 0) return 0;
  int hp = p->hp;
  if (hp < 0) hp = 0;
  if (hp > p->max_hp) hp = p->max_hp;
  int scaled = (hp * 1000) / p->max_hp;  // 0..1000
  if (scaled > 1000) scaled = 1000;
  int16_t packed = (int16_t)(scaled & 0x3FF);  // lower 10 bits
  return packed;                               // upper 6 bits zero for now
}

// Status & volatile bits; limited by what current engine tracks.
static inline int16_t pack_status_and_volatiles(Player* player,
                                                int poke_index) {
  Pokemon* p = &player->team[poke_index];
  int16_t packed = 0;
  // Core status
  packed |= (p->status.paralyzed & 0x1) << 0;      // PAR
  packed |= (p->status.burn & 0x1) << 1;           // BRN
  packed |= (p->status.freeze & 0x1) << 2;         // FRZ
  packed |= (p->status.poison & 0x1) << 3;         // PSN (regular)
  packed |= ((p->status.sleep > 0) ? 1 : 0) << 4;  // SLP
  // Toxic (badly poisoned) tracked via active battle pokemon counter
  if (poke_index == player->active_pokemon_index) {
    if (player->active_pokemon.badly_poisoned_ctr > 0) {
      packed |= 1 << 5;  // TOX
    }
    // if (player->active_pokemon.confusion_counter > 0) {
    //   packed |= 1 << 6; // CONFUSED
    // }
    // if (player->active_pokemon.flinch) {
    //   packed |= 1 << 14; // FLINCH
    // }
  }
  // Bits for SEEDED / SUB / RAGE / DISABLED / PARTIALLY_TRAPPED / TRANSFORMED /
  // FOCUS_ENERGY Not currently implemented in structs; remain 0 as
  // placeholders.
  return packed;
}

static inline void pack_poke(int16_t* row,
                             Player* player,
                             int player_index,
                             int poke_index,
                             int is_active,
                             int hidden) {
  if (hidden) {
    // for (int i = 0; i < PACK_POKE_INTS; i++) row[i] = 0;
    row[5] = 1000;
    return;
  }

  Pokemon* poke = &player->team[poke_index];
  // species id (negated if active)
  int16_t species = (int16_t)poke->id;
  if (is_active) species = (int16_t)(-species);
  row[0] = species;

  // If all four moves have zero or negative PP, report them as Struggle
  int all_zero_pp = 1;
  // For active pokemon, check PP in active_pokemon.moves array
  Move* move_array =
      is_active ? player->active_pokemon.moves : poke->poke_moves;
  for (int k = 0; k < 4; k++) {
    Move* m = &move_array[k];
    if (m->pp > 0) {
      all_zero_pp = 0;
      break;
    }
  }

  // Pack moves: use active_pokemon.moves if this is the active pokemon
  int reveal = (player_index == 1);
  for (int k = 0; k < 4; k++) {
    Move* m = &move_array[k];
    reveal = reveal || (m->revealed);
    if (reveal) {
      if (all_zero_pp) {
        // pack a temporary move as Struggle with 0 PP so we don't mutate state
        Move tmp = *m;
        tmp.id = STRUGGLE_MOVE_ID;
        tmp.pp = 0;
        row[1 + k] = pack_move(&tmp, 0);
      } else {
        row[1 + k] = pack_move(m, 0);
      }
    } else {
      row[1 + k] = 0;
    }
  }
  row[5] = pack_hp_percent(poke);
  row[6] = pack_status_and_volatiles(player, poke_index);
}

// Pack observation from observer's perspective (opponent's unrevealed Pokemon
// are hidden)
void pack_battle(Battle* b, Player* observer, Player* opponent, int16_t* out) {
  // Header: stat mods for observer (first 2 ints) then opponent (next 2 ints)
  stat_mods* observer_mods = &observer->active_pokemon.stat_mods;
  stat_mods* opponent_mods = &opponent->active_pokemon.stat_mods;
  out[0] = pack_attack_def_specA_specD(observer_mods);
  out[1] = pack_stat_acc_eva(observer_mods);
  out[2] = pack_attack_def_specA_specD(opponent_mods);
  out[3] = pack_stat_acc_eva(opponent_mods);

  // Determine which player index (1 or 2) to use for gating revealed moves
  int observer_index = (observer == &b->p1) ? 1 : 2;
  int opponent_index = (observer == &b->p1) ? 2 : 1;

  // Pokémon rows (interleaved: observer slot 0, opponent slot 0, observer slot
  // 1, ...)
  for (int slot = 0; slot < PACK_TEAM_SLOTS; slot++) {
    // Observer's Pokemon (always fully visible)
    int interleave_index_observer = slot * 2 + 0;
    int base_offset_observer =
        PACK_HEADER_INTS + interleave_index_observer * PACK_POKE_INTS;
    pack_poke(out + base_offset_observer,
              observer,
              observer_index,
              slot,
              (slot == observer->active_pokemon_index),
              0);  // not hidden

    // Opponent's Pokemon (hidden if not revealed)
    int hidden = !(observer->shown_pokemon & (1u << slot));
    int interleave_index_opponent = slot * 2 + 1;
    int base_offset_opponent =
        PACK_HEADER_INTS + interleave_index_opponent * PACK_POKE_INTS;
    pack_poke(out + base_offset_opponent,
              opponent,
              opponent_index,
              slot,
              (slot == opponent->active_pokemon_index),
              hidden);
  }
}

// Pack observations for all agents (up to 2 agents)
void pack_all_agents(Battle* b, int num_agents, int16_t* out) {
  // Pack from P1's perspective
  pack_battle(b, &b->p1, &b->p2, out);

  if (num_agents == 2) {
    // Pack from P2's perspective
    pack_battle(b, &b->p2, &b->p1, out + PACK_TOTAL_INTS);
  }
}

#endif