#ifndef SIM_H
#define SIM_H

#include "data_sim/ou_teams.h"
#include "data_sim/typing.h"
#include "sim_logging.h"
#include "sim_packing.h"
#include "sim_utils/battle.h"
#include "sim_utils/battle_queue.h"
#include "sim_utils/move.h"
#include "sim_utils/pokegen.h"
#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"

typedef enum {
  ONE_V_ONE = 0,
  TWO_V_TWO,
  SIX_V_SIX,
  GEN_1_OU,
  TEAM_CONFIG_MAX
} TeamConfig;

typedef struct {
  Log log;
  int16_t* observations;
  int* actions;
  float* rewards;
  unsigned char* terminals;
  int gametype;
  Battle* battle;
  int tick;
  int episode_valid_moves;
  int episode_invalid_moves;
  float accumulated_invalid_penalty;  // Sum of penalties from consecutive
                                      // invalid moves
} Sim;

int valid_choice(int player_num, Player p, unsigned int input, int mode) {
  // The players input doesn't even matter
  if (!(mode == player_num || mode == 3 || mode == 0)) {
    return 1;
  }
  if (input < 6) {
    return valid_switch(p, input);
  }
  if (mode == 0 && input <= 9) {
    return valid_move(&p, input - 6);
  }
  return 0;
}
void action(Battle* b, Player* user, Player* target, int input, int type) {
  if (input >= 6) {
    input -= 6;
    add_move_to_queue(b, user, target, input);
  } else {
    add_switch(b, user, input, type);
  }
  b->action_queue.q_size++;
}

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

// Reward function: Only returns a terminal win/loss signal.
// +1 if player 1 wins, -1 if player 1 loses, 0 otherwise.
// No dense shaping by HP difference.
float reward(Sim* s) {
  Battle* b = s->battle;
  int p1_alive = 0;
  int p2_alive = 0;
  for (int j = 0; j < NUM_POKE; j++) {
    p1_alive += b->p1.team[j].hp > 0 ? 1 : 0;
    p2_alive += b->p2.team[j].hp > 0 ? 1 : 0;
  }
  if (p1_alive == 0) {
    return -1.0f;  // All of player 1's Pokemon fainted => loss
  }
  if (p2_alive == 0) {
    return 1.0f;  // All of player 2's Pokemon fainted => win
  }
  return 0.0f;  // Non-terminal
}

void team_generator(Player* p, TeamConfig config) {
  // Clear the entire Pokemon table
  memset(p->team, 0, sizeof(Pokemon) * NUM_POKE);
  // Reset visibility bitfield
  p->shown_pokemon = 0;

  // Load NUM_POKE pokemon for the team
  if (config == GEN_1_OU) {
    load_team_from_ou(p, -1);  // Load a random OU team
  } else {
    int num_poke = 1;
    if (config == ONE_V_ONE) {
      num_poke = 1;
    } else if (config == TWO_V_TWO) {
      num_poke = 2;
    } else if (config == SIX_V_SIX) {
      num_poke = 6;
    }
    for (int i = 0; i < num_poke; i++) {
      load_pokemon(
          &p->team[i], NULL, 0);  // Load same pokemon for all slots for now
    }
  }
  // Set up active pokemon
  // Initialize and set up active pokemon
  memset(&p->active_pokemon, 0, sizeof(BattlePokemon));
  p->active_pokemon.pokemon = &p->team[0];
  p->active_pokemon_index = 0;
  p->active_pokemon.type1 = p->active_pokemon.pokemon->type1;
  p->active_pokemon.type2 = p->active_pokemon.pokemon->type2;
  // Copy base Pokemon stats and moves into the active slot
  p->active_pokemon.stats = p->active_pokemon.pokemon->stats;
  for (int m = 0; m < 4; ++m) {
    p->active_pokemon.moves[m] = p->active_pokemon.pokemon->poke_moves[m];
  }
  // Mark the active pokemon as seen
  p->shown_pokemon |= (1u << p->active_pokemon_index);
}

// Helper function to get AI player choice
// Does the given player need to switch based on the current mode?

static inline int select_best_move_choice(Player* user, Player* opponent) {
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
static inline int select_valid_switch_choice(Player p) {
  for (int i = 0; i < NUM_POKE; ++i) {
    if (valid_switch(p, i)) return i;
  }
  // Crash state
  return 0;
}

// Helper function to get AI player choice
static inline int get_p2_choice(Sim* s, int mode) {
  Battle* b = s->battle;
  if (mode == 3 || mode == 2) {
    return select_valid_switch_choice(b->p2);
  }
  // Regular mode: choose best damaging move
  int action = select_best_move_choice(&b->p2, &b->p1);

  int num_failed = 10;
  while (!(valid_choice(2, b->p2, action, mode))) {
    action = select_best_move_choice(&b->p2, &b->p1);
    num_failed--;
    if (num_failed <= 0) {
      DLOG("Failed to properly choose move - checking all actions");
      // Try all possible actions (switches and moves)
      for (int i = 0; i < 10; i++) {
        if (valid_choice(2, b->p2, i, mode)) {
          return i;
        }
      }
      // No valid moves available - signal this to the caller
      DLOG("No valid moves found for opponent");
      return -1;
    }
  }
  return action;
  // return select_best_move_choice(&b->p2);
}

// Helper function to check if a player can act (not frozen/sleeping unless
// switching)
static inline int can_player_act(Player* player, int choice) {
  return (!player->active_pokemon.pokemon->status.freeze &&
          !player->active_pokemon.pokemon->status.sleep) ||
         choice < NUM_POKE;  // Switch moves bypass status
}

// Helper function to handle regular battle mode (no fainted Pokemon)
static inline void handle_regular_mode(Battle* b,
                                       int p1_choice,
                                       int p2_choice) {
  if (can_player_act(&b->p1, p1_choice)) {
    action(b, &b->p1, &b->p2, p1_choice, REGULAR);
  }
  if (can_player_act(&b->p2, p2_choice)) {
    action(b, &b->p2, &b->p1, p2_choice, REGULAR);
  }
}

// Helper function to handle fainted Pokemon mode
static inline void handle_fainted_mode(Battle* b,
                                       int mode,
                                       int p1_choice,
                                       int p2_choice) {
  if (mode == 1 || mode == 3) {
    // Player 1 needs to switch
    action(b, &b->p1, &b->p2, p1_choice, FAINTED);
  }
  if (mode == 2 || mode == 3) {
    // Player 2 needs to switch
    action(b, &b->p2, &b->p1, p2_choice, FAINTED);
  }
}

static inline int battle_step(Sim* sim, int choice, PrevChoices* prev) {
  Battle* b = sim->battle;
  int mode = b->mode;
  int p1_choice = choice;

  // Get AI player choice
  int p2_choice = get_p2_choice(sim, mode);
  // Validate player 1's choice
  if (!valid_choice(1, b->p1, p1_choice, mode)) {
    return -1;
  }
  if (p2_choice < 0 || !valid_choice(2, b->p2, p2_choice, mode)) {
    return -2;
  }

  // Encode p1
  if (p1_choice < 6) {
    prev->p1_choice = 1;
    prev->p1_val = choice;
  } else if (p1_choice >= 6 && p1_choice < 10) {
    int mi = p1_choice - 6;
    Move* mv = &b->p1.active_pokemon.pokemon->poke_moves[mi];
    prev->p1_choice = 2;
    prev->p1_val = mv ? (int)mv->id : -1;
  }
  // Encode p2
  if (p2_choice < 6) {
    prev->p2_choice = 1;
    prev->p2_val = p2_choice;
  } else if (p2_choice >= 6 && p2_choice < 10) {
    int mi2 = p2_choice - 6;
    Move* mv2 = &b->p2.active_pokemon.pokemon->poke_moves[mi2];
    prev->p2_choice = 2;
    prev->p2_val = mv2 ? (int)mv2->id : -1;
  }
  // (Future: can optionally log opponent choice into a ring buffer if needed)

  // Handle actions based on battle mode
  if (mode == 0) {
    handle_regular_mode(b, p1_choice, p2_choice);
  } else {
    handle_fainted_mode(b, mode, p1_choice, p2_choice);
  }

  // Process the battle queue and update mode
  mode = eval_queue(b);
  b->mode = mode;
  return mode;
}

void reset_sim(Sim* s) {
  Battle* b = s->battle;
  // Dealing with players is already handled by the team generator
  b->action_queue.q_size = 0;
  b->turn_num = 0;
  b->lastMove = NULL;
  b->lastDamage = 0;
  b->mode = 0;

  s->tick = 0;
  s->rewards[0] = 0.0f;
  s->episode_valid_moves = 0;
  s->episode_invalid_moves = 0;
  s->accumulated_invalid_penalty = 0.0f;
  return;
}

void c_reset(Sim* sim) {
  if (!sim->battle) {
    sim->battle = (Battle*)calloc(1, sizeof(Battle));
    // Initialize all log metrics to zero
  } else {
    //Todo: Add an object in log_episode which I can pack and send over so it's a lot easier
    log_episode(&sim->log,
                sim->battle,
                sim->rewards[0],
                sim->episode_valid_moves,
                sim->episode_invalid_moves,
                sim->tick,
                sim->gametype - SIX_V_SIX);
    reset_sim(sim);
  }
  // TeamConfig config = rand() % TEAM_CONFIG_MAX;
  TeamConfig config = rand() % 2 + SIX_V_SIX;
  sim->gametype = (int)config;
 team_generator(&sim->battle->p1, config);
  team_generator(&sim->battle->p2, config);

  pack_battle(sim->battle, sim->observations);
}
// No rendering: bare text
void c_render(Sim* sim) { return; }

void c_close(Sim* sim) {
  if (sim->battle) {
    free(sim->battle);  // Frees the entire slab (Battle + Teams + Moves)
    sim->battle = NULL;
  }
}

void c_step(Sim* sim) {
  // Reset, return battle state, and reset
  if (sim->terminals[0]) {
    c_reset(sim);
    // Reset terminal flag at start of step so model can observe if game ended
    sim->terminals[0] = 0;
    pack_battle(sim->battle, sim->observations);
    return;
  }

  sim->tick++;
  Battle* battle = sim->battle;

  int raw_choice = sim->actions[0];
  // Capture active pokemon indices and HP before resolving this step
  PrevChoices step_prev = {0};
  int a = battle_step(sim, raw_choice, &step_prev);
  if (a == -2) {
    // opponent ran out of valid moves - treat as terminal (This shouldn't
    // happen ever thanks to struggle)
    sim->rewards[0] = 0.0f;
    sim->terminals[0] = 1;
    // pack_battle(battle, sim->observations);
    return;
  }
  if (a == -1) {
    // Invalid move penalty (shaping only for invalid action): -0.01
    sim->rewards[0] = -0.01f;
    sim->episode_invalid_moves += 1;
    if (sim->accumulated_invalid_penalty < 0.5f) {
      sim->accumulated_invalid_penalty += 0.01f;  // Accumulate the penalty
    }
    pack_battle(battle, sim->observations);
    return;
  }
  sim->episode_valid_moves += 1;
  battle->mode = a;
  if (a == 0) {
    battle->mode = end_step(battle);
  }
  // No end step if a pokemon has fainted (gen1 quirk)
  battle->action_queue.q_size = 0;

  float r = reward(sim);
  if (r == 1.0f || r == -1.0f) {
    sim->rewards[0] = r;  // Terminal reward overrides any bonus
    sim->terminals[0] = 1;
  } else {
    sim->rewards[0] = sim->accumulated_invalid_penalty;
    sim->accumulated_invalid_penalty = 0.0f;  // Reset the accumulator
  }

  pack_battle(sim->battle, sim->observations);
  return;
}

#endif
