#ifndef OPPONENT_BEHAVIOR_H
#define OPPONENT_BEHAVIOR_H

#include <stdlib.h>

#include "../data_sim/typing.h"
#include "battle.h"
#include "move.h"
#include "switch.h"

// Forward declarations
int valid_choice(int player_num, Player p, unsigned int input, int mode);

// Get highest damage move index (simple damage calculation)
int get_highest_damage_move_index(Player* player) {
  BattlePokemon* active_pokemon = &player->active_pokemon;
  int best_move_index = -1;
  int max_damage = -1;
  // Iterate through the Pokémon's moves
  for (int i = 0; i < 4; i++) {
    Move* move = &active_pokemon->pokemon->poke_moves[i];
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
int select_best_move_choice(Player* user, Player* opponent) {
  BattlePokemon* opponent_active = &opponent->active_pokemon;

  BattlePokemon* active_pokemon = &user->active_pokemon;
  int best_move_index = -1;
  float max_effective_damage = -1.0f;

  // Iterate through the Pokémon's moves and score them. Include STAB (1.5x)
  // for moves whose type matches the user's type1 or type2.
  for (int i = 0; i < 4; i++) {
    Move* move = &active_pokemon->pokemon->poke_moves[i];
    if (!move) continue;
    // Skip moves with no PP or no base power (status moves)
    if (move->pp <= 0 || move->power <= 0) continue;

    float base_damage = (float)move->power;
    // Calculate type effectiveness against opponent's types
    float type_mod1 = damage_chart[move->type][opponent_active->type1];
    float type_mod2 = damage_chart[move->type][opponent_active->type2];
    float total_type_mod = type_mod1 * type_mod2;

    // STAB: 1.5x if move type matches either of the user's types
    float stab = 1.0f;
    if (move->type == active_pokemon->type1 ||
        move->type == active_pokemon->type2) {
      stab = 1.5f;
    }

    // Effective damage = base power × type effectiveness × stab
    float effective_damage = base_damage * total_type_mod * stab;
    // Penalize moves with a recharge phase (Hyper Beam, Solar Beam)
    if (move->id == HYPER_BEAM_MOVE_ID || move->id == SOLAR_BEAM_MOVE_ID) {
      effective_damage *= 0.5f;
    }

    // small tie-breaker: prefer higher PP remaining
    effective_damage += ((float)move->pp) * 0.001f;

    if (effective_damage > max_effective_damage) {
      max_effective_damage = effective_damage;
      best_move_index = i;
    }
  }
  if (best_move_index >= 0) {
    return 6 + best_move_index;  // encode as move input [6..9]
  }
  // Fallback: random move
  return 6 + (rand() % 4);
}

// Select a valid switch index [0..NUM_POKE-1]
int select_valid_switch_choice(Player p) {
  for (int i = 0; i < NUM_POKE; ++i) {
    if (valid_switch(p, i)) return i;
  }
  // Crash state
  return 0;
}

#endif
