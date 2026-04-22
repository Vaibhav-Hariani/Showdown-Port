#ifndef SIM_H
#define SIM_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "data_sim/ou_teams.h"
#include "data_sim/typing.h"
#include "sim_utils/sim_logging.h"
#include "sim_utils/sim_packing.h"
#include "sim_utils/battle.h"
#include "sim_utils/battle_queue.h"
#include "sim_utils/move.h"
#include "sim_utils/opponent_behavior.h"
#include "sim_utils/pokegen.h"

static inline int get_team_index(const Player* player, const Pokemon* pokemon);

typedef enum { RANDOM_AI = 1, MAX_DAMAGE_AI = 2, GEN1_AI = 3 } OpponentType;

typedef struct {
  Log log;
  int16_t* observations;
  int* actions;
  float* rewards;
  unsigned char* terminals;
  int num_agents;   // Number of agents (1 = policy vs AI, 2 = both externally controlled)
  int opp_type;     // If num_agents==1, choose fixed AI: 1=random, 2=max-power, 3=gen1
  int max_gametype; // Curriculum cap: sample from TeamConfig [0, max_gametype-1]
  int gametype;
  Battle* battle;
  int tick;
  int episode_valid_moves;
  int episode_invalid_moves;
  float accumulated_invalid_penalty[2];
} Sim;

// Forward declarations for functions defined later
void c_reset(Sim* sim);
static inline void set_active(Player* p);

void sim_init(Sim* sim) {
  sim_srand((unsigned int)rand());
  sim->battle = (Battle*)calloc(1, sizeof(Battle));
  if (sim->num_agents < 1 || sim->num_agents > 2) {
    sim->num_agents = 1;
  }
  if (sim->opp_type < RANDOM_AI || sim->opp_type > GEN1_AI) {
    sim->opp_type = GEN1_AI;
  }
  if (sim->max_gametype < 1 || sim->max_gametype > TEAM_CONFIG_MAX) {
    sim->max_gametype = TEAM_CONFIG_MAX;
  }
  c_reset(sim);
}

int valid_choice(int player_num, const Player* p, unsigned int input, int mode) {
  // If this player is not expected to act this step, treat as valid.
  if (mode != 0 && mode != 3 && mode != player_num) {
    return 1;
  }
  if (input < NUM_POKE) {
    return valid_switch(p, input);
  }
  if (mode == 0 && input <= 9) {
    return valid_move(p, input - 6);
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
}

// Reward function: Only returns a terminal win/loss signal.
// +1 if player 1 wins, -1 if player 1 loses, 0 otherwise.
// No dense shaping by HP difference.
float reward(Sim* s) {
  Battle* b = s->battle;
  int p1_alive = 0;
  int p2_alive = 0;
  for (int j = 0; j < NUM_POKE; j++) {
    if (!p1_alive && b->p1.team[j].hp > 0) {
      p1_alive = 1;
    }
    if (!p2_alive && b->p2.team[j].hp > 0) {
      p2_alive = 1;
    }
    // Common path: both sides still have live Pokemon.
    if (p1_alive && p2_alive) {
      return 0.0f;
    }
  }
  if (p1_alive == 0) {
    return -1.0f;  // All of player 1's Pokemon fainted => loss
  }
  if (p2_alive == 0) {
    return 1.0f;  // All of player 2's Pokemon fainted => win
  }
  return 0.0f;  // Non-terminal
}

static inline void set_active(Player* p) {
  // Set up active pokemon
  // Initialize and set up active pokemon
  p->active_pokemon.pokemon = &p->team[0];
  p->active_pokemon_index =
      (char)get_team_index(p, p->active_pokemon.pokemon);
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

void team_generator(Player* p, TeamConfig config) {
  // Reset visibility bitfield
  p->shown_pokemon = 0;

  // Load NUM_POKE pokemon for the team
  if (config == GEN_1_OU) {
    // OU loader overwrites all six slots, so no full memset is needed.
    load_team_from_ou(p, -1);  // Load a random OU team
  } else {
    // Non-OU configs may leave trailing slots unused; clear table first.
    memset(p->team, 0, sizeof(Pokemon) * NUM_POKE);
    int num_poke = 1;
    if (config == ONE_V_ONE) {
      num_poke = 1;
    } else if (config == TWO_V_TWO) {
      num_poke = 2;
    } else if (config == SIX_V_SIX) {
      num_poke = 6;
    }
    for (int i = 0; i < num_poke; i++) {
      load_pokemon(&p->team[i],
                   NULL,
                   0,
                   MISSINGNO);  // Load same pokemon for all slots for now
    }
  }
  set_active(p);
}

// Helper function to check if a player can act (not frozen/sleeping unless
// switching)
static inline int can_player_act(Player* player, int choice) {
  return (!player->active_pokemon.pokemon->status.freeze &&
          !player->active_pokemon.pokemon->status.sleep) ||
         choice < NUM_POKE;  // Switch moves bypass status
}

// Helper function to get AI player choice (P2)
static inline int ai_choice(Sim* sim, int mode) {
  Battle* b = sim->battle;
  int action = 0;
  if (sim->opp_type == RANDOM_AI) {
    if (mode == 2 || mode == 3) {
      action = select_valid_switch_choice(&b->p2);
    } else {
      int move_candidates[4] = {0};
      int move_count = 0;
      for (int i = 6; i <= 9; i++) {
        if (valid_choice(2, &b->p2, (unsigned int)i, mode)) {
          move_candidates[move_count++] = i;
        }
      }
      if (move_count > 0) {
        action = move_candidates[sim_rand_bounded(move_count)];
      }
    }
  } else if (sim->opp_type == MAX_DAMAGE_AI) {
    if (mode == 2 || mode == 3) {
      action = select_valid_switch_choice(&b->p2);
    } else {
      int best_move = get_highest_damage_move_index(&b->p2);
      if (best_move >= 0) {
        int best_choice = 6 + best_move;
        if (valid_choice(2, &b->p2, (unsigned int)best_choice, mode)) {
          action = best_choice;
        }
      }
    }
  } else {
    action = choose_gen1_ai_action(2, &b->p2, &b->p1, mode);
  }

  if (action == 0 || !valid_choice(2, &b->p2, (unsigned int)action, mode)) {
    action = choose_gen1_ai_action(2, &b->p2, &b->p1, mode);
  }
  if (action == 0 && !valid_choice(2, &b->p2, 0, mode)) {
    DLOG("No valid actions found for opponent");
  }
  return action;
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

static inline int battle_step(Battle* b, int p1_choice, int p2_choice) {
  int mode = b->mode;
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
  // Reset rewards and terminals for all agents
  for (int i = 0; i < s->num_agents; i++) {
    s->rewards[i] = 0.0f;
    s->terminals[i] = 0;
    s->accumulated_invalid_penalty[i] = 0.0f;
  }
  s->episode_valid_moves = 0;
  s->episode_invalid_moves = 0;
}

void c_reset(Sim* sim) {
  log_episode(&sim->log,
              sim->battle,
              sim->rewards[0],
              sim->episode_valid_moves,
              sim->episode_invalid_moves,
              sim->tick,
              sim->gametype);
  reset_sim(sim);
  // Curriculum: sample uniformly from [ONE_V_ONE, max_gametype-1].
  TeamConfig config = (TeamConfig)sim_rand_bounded(sim->max_gametype);
  sim->gametype = (int)config;
  team_generator(&sim->battle->p1, config);
  team_generator(&sim->battle->p2, config);

  // Pack observations for all agents
  pack_all_agents(sim->battle, sim->num_agents, sim->observations);
}

// No rendering: bare text
void c_render(Sim* sim) { return; }

void c_close(Sim* sim) { free(sim->battle); }

// c_step validates inputs, executes battle step, and cleans up
void c_step(Sim* sim) {
  // Reset if terminal, return battle state
  if (sim->terminals[0]) {
    c_reset(sim);
    for (int i = 0; i < sim->num_agents; i++) {
      sim->terminals[i] = 0;
    }
    return;
  }

  sim->tick++;
  Battle* battle = sim->battle;

  int selfplay = (sim->num_agents == 1);
  int mode = battle->mode;

  int raw_choice_p1 = sim->actions[0];
  int raw_choice_p2 = selfplay ? ai_choice(sim, mode) : sim->actions[1];

  if (!valid_choice(1, &battle->p1, raw_choice_p1, mode)) {
    // Invalid move penalty for P1
    sim->rewards[0] = -0.01f;
    sim->episode_invalid_moves += 1;
    if (sim->accumulated_invalid_penalty[0] < 0.5f) {
      sim->accumulated_invalid_penalty[0] += 0.01f;
    }
    pack_all_agents(battle, sim->num_agents, sim->observations);
    return;
  }
  if (!selfplay && !valid_choice(2, &battle->p2, raw_choice_p2, mode)) {
    sim->rewards[1] = -0.01f;
    sim->episode_invalid_moves += 1;
    if (sim->accumulated_invalid_penalty[1] < 0.5f) {
      sim->accumulated_invalid_penalty[1] += 0.01f;
    }
    pack_all_agents(battle, sim->num_agents, sim->observations);
    return;
  }

  // This is the core sim body: everything around this is puffer support

  // Both choices valid - execute battle step
  int a = battle_step(battle, raw_choice_p1, raw_choice_p2);
  sim->episode_valid_moves += 1;
  battle->mode = a;

  // No end step if a pokemon has fainted (gen1 quirk)
  if (a == 0) {
    battle->mode = end_step(battle);
  }

  // End of sim

  battle->action_queue.q_size = 0;
  float r = 0.0f;
  if (battle->mode != 0) {
    r = reward(sim);
  }
  if (r == 1.0f || r == -1.0f) {
    // Terminal: P1 wins (+1) or loses (-1)
    sim->rewards[0] = r;
    sim->terminals[0] = 1;
    if (!selfplay) {
      sim->rewards[1] = -r;  // P2 gets opposite reward
      sim->terminals[1] = 1;
    }
  } else {
    // Non-terminal: apply accumulated penalty per agent
    sim->rewards[0] = sim->accumulated_invalid_penalty[0];
    sim->accumulated_invalid_penalty[0] = 0.0f;
    if (!selfplay) {
      sim->rewards[1] = sim->accumulated_invalid_penalty[1];
      sim->accumulated_invalid_penalty[1] = 0.0f;
    }
  }

  // Pack observations for all agents
  pack_all_agents(battle, sim->num_agents, sim->observations);
  return;
}

#endif
