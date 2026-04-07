#ifndef OPPONENT_BEHAVIOR_H
#define OPPONENT_BEHAVIOR_H

#include <stdlib.h>

#include "../data_sim/typing.h"
#include "battle.h"
#include "move.h"
#include "switch.h"

// Forward declarations
int valid_choice(int player_num, const Player* p, unsigned int input, int mode);

// Get highest damage move index (simple damage calculation)
static inline int get_highest_damage_move_index(Player* player) {
  BattlePokemon* active_pokemon = &player->active_pokemon;
  int best_move_index = -1;
  int max_damage = -1;
  // Iterate through the Pokémon's moves
  for (int i = 0; i < 4; i++) {
    Move* move = &active_pokemon->moves[i];
    // Check if the move has PP left
    if (move->pp > 0) {
      int damage = move->power;  // Simplified damage calculation
      // Update the best move if this move has higher damage
      if (damage > max_damage) {
        max_damage = damage;
        best_move_index = i;
      }
    }
  }
  return best_move_index;
}

// Select best move considering type effectiveness, STAB, and move power
static inline int gen1_ai_move(Player* user, Player* opponent) {
  BattlePokemon* opponent_active = &opponent->active_pokemon;

  BattlePokemon* active_pokemon = &user->active_pokemon;

  int best_move_index = -1;
  uint32_t max_effective_damage = 0;

  // Iterate through the Pokémon's moves and score them. Include STAB (1.5x)
  // for moves whose type matches the user's type1 or type2.
  for (int i = 0; i < 4; i++) {
    Move* move = &active_pokemon->moves[i];
    if (!move || move->pp <= 0 || move->power <= 0) continue;

    uint32_t base_damage = (uint32_t)move->power;
    // Calculate type effectiveness against opponent's types using fixed-point
    // damage_chart values are uint16_t where 256 = 1.0x
    uint32_t type_mod1 = damage_chart[move->type][opponent_active->type1];
    uint32_t type_mod2 = damage_chart[move->type][opponent_active->type2];
    // Multiply type modifiers: (type_mod1 * type_mod2) / 256
    uint32_t total_type_mod = (type_mod1 * type_mod2) >> 8;

    // STAB: 384 (1.5x) if move type matches either of the user's types, else 256 (1.0x)
    uint16_t stab = 256;
    if (move->type == active_pokemon->type1 ||
        move->type == active_pokemon->type2) {
      stab = 384;
    }

    // Effective damage = base power × type effectiveness × stab
    // Result is in fixed-point, divide by 256 at the end
    uint32_t effective_damage = (base_damage * total_type_mod * stab) >> 8;
    
    // Custom penalty for recharge moves (Hyper Beam, Solar Beam): multiply by 0.5
    // In fixed-point: multiply by 128 (0.5 * 256) and divide by 256
    if (move->id == HYPER_BEAM_MOVE_ID || move->id == SOLAR_BEAM_MOVE_ID) {
      effective_damage = (effective_damage * 128) >> 8;
    }
    if (effective_damage > max_effective_damage) {
      max_effective_damage = effective_damage;
      best_move_index = i;
    }
  }
  if (best_move_index >= 0) {
    return 6 + best_move_index;  // encode as move input [6..9]
  }
  // Fallback: random move
  return 6 + (sim_rand() % 4);
}

// Select a valid switch index [0..NUM_POKE-1]
static inline int select_valid_switch_choice(const Player* p) {
  for (int i = 0; i < NUM_POKE; ++i) {
    if (valid_switch(p, i)) return i;
  }
  // Crash state
  return 0;
}

// Select an action input for the current mode using the Gen1 heuristic.
// Returns either switch slot [0..NUM_POKE-1] or move input [6..9].
static inline int choose_gen1_ai_action(int player_num,
                                        Player* user,
                                        Player* opponent,
                                        int mode) {
  if (mode == player_num || mode == 3) {
    return select_valid_switch_choice(user);
  }

  // Gen1-style preference: if possible, repeat the last used move.
  Move* last = user->active_pokemon.last_used;
  if (last != NULL) {
    int idx = last - user->active_pokemon.moves;
    if (idx < 4) {
      int repeat_choice = 6 + (int)idx;
      if (valid_choice(player_num, user, (unsigned int)repeat_choice, mode)) {
        return repeat_choice;
      }
    }
  }

  int choice = gen1_ai_move(user, opponent);
  if (valid_choice(player_num, user, (unsigned int)choice, mode)) {
    return choice;
  }

  for (int i = 6; i <= 9; i++) {
    if (valid_choice(player_num, user, (unsigned int)i, mode)) {
      return i;
    }
  }

  for (int i = 0; i < NUM_POKE; i++) {
    if (valid_choice(player_num, user, (unsigned int)i, mode)) {
      return i;
    }
  }

  return 0;
}

#endif
