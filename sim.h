#ifndef SIM_H
#define SIM_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "data_sim/ou_teams.h"
#include "data_sim/typing.h"
#include "sim_logging.h"
#include "sim_packing.h"
#include "sim_utils/battle.h"
#include "sim_utils/battle_queue.h"
#include "sim_utils/move.h"
#include "sim_utils/opponent_behavior.h"
#include "sim_utils/pokegen.h"

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
  int num_agents;  // Number of agents (1 = P1 only, 2 = both P1 and P2)
  TeamConfig gametype;
  Battle* battle;
  int tick;
  int episode_valid_moves;
  int episode_invalid_moves;
  float accumulated_invalid_penalty[2];  // Per-agent: sum of penalties from
                                         // consecutive invalid moves
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

// Helper function to check if a player can act (not frozen/sleeping unless
// switching)
static inline int can_player_act(Player* player, int choice) {
  return (!player->active_pokemon.pokemon->status.freeze &&
          !player->active_pokemon.pokemon->status.sleep) ||
         choice < NUM_POKE;  // Switch moves bypass status
}

// Helper function to get AI player choice (P2)
static inline int get_p2_choice(Sim* sim, int mode) {
  Battle* b = sim->battle;
  int action = 6 + (rand() % 4);
  // GEN_1_OU: original behavior (strategic AI)
  if (mode == 3 || mode == 2) {
    return select_valid_switch_choice(b->p2);
  }
  // Regular mode: choose best damaging move
  if (mode == 0) {
    // if (sim->gametype == GEN_1_OU) {
    //   action = select_best_move_choice(&b->p2, &b->p1);
    // }
    int num_failed = 10;
    while (!(valid_choice(2, b->p2, action, mode)) && num_failed > 0) {
      action = 6 + (rand() % 4);
      num_failed--;
    }
    if (num_failed <= 0) {
      DLOG("Failed to properly choose move - checking all actions");
      for (int i = 9; i >= 0; i--) {
        if (valid_choice(2, b->p2, i, mode)) {
          return i;
        }
      }
      DLOG("No valid moves found for opponent");
      return -1;
    }
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

static inline int battle_step(Sim* sim, int p1_choice, int p2_choice) {
  Battle* b = sim->battle;
  int mode = b->mode;

  // Validate player choices
  if (!valid_choice(1, b->p1, p1_choice, mode)) {
    return -1;
  }
  if (p2_choice < 0 || !valid_choice(2, b->p2, p2_choice, mode)) {
    return -2;
  }
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
  return;
}

init_sim(Sim* sim){
  if (!sim->battle) {
    sim->battle = (Battle*)calloc(1, sizeof(Battle));
  }
  reset_sim(sim);
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
  TeamConfig config = rand() % TEAM_CONFIG_MAX;
  sim->gametype = (int)config;
  team_generator(&sim->battle->p1, config);
  team_generator(&sim->battle->p2, config);

  // Pack observations for all agents
  pack_all_agents(sim->battle, sim->num_agents, sim->observations);
}

// No rendering: bare text
void c_render(Sim* sim) { return; }

void c_close(Sim* sim) {
  if (sim->battle) {
    free(sim->battle);
    sim->battle = NULL;
  }
}

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

  int raw_choice_p1 = sim->actions[0];
  int raw_choice_p2;

  // Get P2 choice: from policy if num_agents==2, else from AI
  if (sim->num_agents == 2) {
    raw_choice_p2 = sim->actions[1];
  } else {
    raw_choice_p2 = get_p2_choice(sim, battle->mode);
  }

  if (!valid_choice(1, battle->p1, raw_choice_p1, battle->mode)) {
    // Invalid move penalty for P1
    sim->rewards[0] = -0.01f;
    sim->episode_invalid_moves += 1;
    if (sim->accumulated_invalid_penalty[0] < 0.5f) {
      sim->accumulated_invalid_penalty[0] += 0.01f;
    }
    pack_all_agents(battle, sim->num_agents, sim->observations);
    return;
  }
  if (raw_choice_p2 < 0 ||
      !valid_choice(2, battle->p2, raw_choice_p2, battle->mode)) {
    if (sim->num_agents == 2) {
      sim->rewards[1] = -0.01f;
      sim->episode_invalid_moves += 1;
      if (sim->accumulated_invalid_penalty[1] < 0.5f) {
        sim->accumulated_invalid_penalty[1] += 0.01f;
      }
      pack_all_agents(battle, sim->num_agents, sim->observations);
      return;
    } else {
      // AI opponent ran out of valid moves - treat as terminal
      sim->rewards[0] = 0.0f;
      sim->terminals[0] = 1;
      return;
    }
  }
  // Both choices valid - execute battle step
  int a = battle_step(sim, raw_choice_p1, raw_choice_p2);

  sim->episode_valid_moves += 1;
  battle->mode = a;
  if (a == 0) {
    battle->mode = end_step(battle);
  }
  // No end step if a pokemon has fainted (gen1 quirk)
  battle->action_queue.q_size = 0;

  float r = reward(sim);
  if (r == 1.0f || r == -1.0f) {
    // Terminal: P1 wins (+1) or loses (-1)
    sim->rewards[0] = r;
    sim->terminals[0] = 1;
    if (sim->num_agents == 2) {
      sim->rewards[1] = -r;  // P2 gets opposite reward
      sim->terminals[1] = 1;
    }
  } else {
    // Non-terminal: apply accumulated penalty per agent
    sim->rewards[0] = sim->accumulated_invalid_penalty[0];
    sim->accumulated_invalid_penalty[0] = 0.0f;
    if (sim->num_agents == 2) {
      sim->rewards[1] = sim->accumulated_invalid_penalty[1];
      sim->accumulated_invalid_penalty[1] = 0.0f;
    }
  }

  // Pack observations for all agents
  pack_all_agents(battle, sim->num_agents, sim->observations);
  return;
}

#endif
