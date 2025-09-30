#ifndef SIM_H
#define SIM_H

#include "sim_utils/battle.h"
#include "sim_utils/battle_queue.h"
#include "sim_utils/move.h"
#include "sim_utils/pokegen.h"
#include "data_sim/typing.h"
#include "sim_logging.h"
#include "sim_packing.h"
#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"

typedef struct {
  Log log;  // Required field. Env binding code uses this to aggregate logs
  int16_t* observations;  // Required. You can use any obs type, but make sure
                          // it matches in Python!
  int* actions;    // Required. int* for discrete/multidiscrete, float* for box
  float* rewards;  // Required
  unsigned char*
      terminals;  // Required. We don't yet have truncations as standard yet
  Battle* battle;
  int tick;
  // Not strictly necessary,
  //  figure this might make life a bit easier with de-rewarding long running
  //  games.
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
  // Action* cur = (b->action_queue.queue) + b->action_queue.q_size;
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

float reward(Sim* s) {
  Battle* b = s->battle;
  float p1_percent_sum = 0, p2_percent_sum = 0;
  for (int j = 0; j < NUM_POKE; j++) {
    float hp1 = b->p1.team[j].hp;
    p1_percent_sum += hp1 / b->p1.team[j].max_hp;
    
    float hp2 = b->p2.team[j].hp;
    p2_percent_sum += hp2 / b->p2.team[j].max_hp;
  }
  // Calculate mean HP percentage for each team (always NUM_POKE Pokemon)
  float mean_p1 = p1_percent_sum / NUM_POKE;
  float mean_p2 = p2_percent_sum / NUM_POKE;
  if (p1_percent_sum == 0.0f) {
    float avg_dmg = (1 - mean_p2) / s->tick;
    s->log.avg_damage_pct += avg_dmg;
    return -1.0f;
  }
  if (p2_percent_sum == 0.0f) {
    float avg_dmg = 1.0f / s->tick;
    s->log.avg_damage_pct += avg_dmg;
    return 1.0f;
  }
  float result = mean_p1 - mean_p2;
  //Clamping happens in puffeRL anyway
  return result;
}

void team_generator(Player* p) {
  // Clear the entire Pokemon table
  memset(p->team, 0, sizeof(Pokemon) * NUM_POKE);
  
  // Load NUM_POKE pokemon for the team
  for (int i = 0; i < NUM_POKE; i++) {
    load_pokemon(&p->team[i], NULL, 0); // Load same pokemon for all slots for now
  }
  
  // Set up active pokemon
  p->active_pokemon.pokemon = &p->team[0];
  p->active_pokemon_index = 0;
  p->active_pokemon.type1 = p->active_pokemon.pokemon->type1;
  p->active_pokemon.type2 = p->active_pokemon.pokemon->type2;
}

// Helper function to get AI player choice
// Does the given player need to switch based on the current mode?


static inline int select_best_move_choice(Player* player) {
  int idx = get_highest_damage_move_index(player); // 0..3 or -1
  if (idx >= 0) {
    return 6 + idx; // encode as move input [6..9]
  }
  // Fallback: random move
  return 6 + (rand() % 4);
}

// Select a valid switch index [0..NUM_POKE-1]
static inline int select_valid_switch_choice(Player p) {
  // Try random first; fall back to first valid if needed
  for (int tries = 0; tries < 32; ++tries) {
    int c = rand() % NUM_POKE;
    if (valid_switch(p, c)) return c;
  }
  for (int i = 0; i < NUM_POKE; ++i) {
    if (valid_switch(p, i)) return i;
  }
  // Should not happen; return 0 as a last resort
  return 0;
}

// Helper function to get AI player choice
static inline int get_p2_choice(Battle* b, int mode) {
  if(mode == 3 || mode == 2) {
    return select_valid_switch_choice(b->p2);
  }
  // Regular mode: choose best damaging move
  return select_best_move_choice(&b->p2);
}

// Helper function to check if a player can act (not frozen/sleeping unless switching)
static inline int can_player_act(Player* player, int choice) {
  return (!player->active_pokemon.pokemon->status.freeze &&
          !player->active_pokemon.pokemon->status.sleep) ||
         choice < NUM_POKE; // Switch moves bypass status
}

// Helper function to handle regular battle mode (no fainted Pokemon)
static inline void handle_regular_mode(Battle* b, int p1_choice, int p2_choice) {
  if (can_player_act(&b->p1, p1_choice)) {
    action(b, &b->p1, &b->p2, p1_choice, REGULAR);
  }
  if (can_player_act(&b->p2, p2_choice)) {
    action(b, &b->p2, &b->p1, p2_choice, REGULAR);
  }
}

// Helper function to handle fainted Pokemon mode
static inline void handle_fainted_mode(Battle* b, int mode, int p1_choice, int p2_choice) {
  if (mode == 1 || mode == 3) {
    // Player 1 needs to switch
    action(b, &b->p1, &b->p2, p1_choice, FAINTED);
  } if (mode == 2 || mode == 3) {
    // Player 2 needs to switch
    action(b, &b->p2, &b->p1, p2_choice, FAINTED);
  }
}

static inline int battle_step(Sim* sim, int choice) {
  Battle* b = sim->battle;
  int mode = b->mode;
  int p1_choice = choice;
  
  // Get AI player choice
  int p2_choice = get_p2_choice(b, mode);
  
  // Validate player 1's choice
  if (!valid_choice(1, b->p1, p1_choice, mode)) {
    log_mini(&sim->log, -1, -1.0f); // Track invalid move with penalty
    return -1;
  }
  
  // Validate player 2's choice (should never fail due to get_p2_choice logic)
  if (!valid_choice(2, b->p2, p2_choice, mode)) {
    return -2; // Error condition
  }
  
  // Track valid move
  log_mini(&sim->log, 1, 0.0f);
  
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

void clear_battle(Battle* b) {
  // Dealing with players is already handled by the team generator
  b->action_queue.q_size = 0;
  b->turn_num = 0;
  b->lastMove = NULL;
  b->lastDamage = 0;
  b->mode = 0;
  return;
}

void c_reset(Sim* sim) {
  if (!sim->battle) {
    sim->battle = (Battle*)calloc(1, sizeof(Battle));
    // Initialize all log metrics to zero
  } else {
    clear_battle(sim->battle);
  }
  sim->tick = 0;
  sim->rewards[0] = 0.0f;
   
  team_generator(&sim->battle->p1);
  team_generator(&sim->battle->p2);

  initial_log(&sim->log, sim->battle);
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
  // Reset terminal flag at start of step so model can observe if game ended
  sim->terminals[0] = 0;
  sim->tick++;
  int a = battle_step(sim, sim->actions[0]);
  if (a == -1) {
    // Sim inputted something invalid; this should be penalized?
    // Otherwise, the sim is incentivized to spam wrong moves to extend the game
    // when it knows it's lost
    sim->rewards[0] = -1.0f;
    pack_battle(sim->battle, sim->observations);
    return;
  }
  sim->battle->mode = a;
  if (a == 0) {
    sim->battle->mode = end_step(sim->battle);
  }
  // No end step if a pokemon has fainted (gen1 quirk). Simply clear the queue
  // and move on
  sim->battle->action_queue.q_size = 0;
  float r = reward(sim);
  if (r == 1.0f || r == -1.0f) {
    final_update(&sim->log, r);
    c_reset(sim);
    sim->terminals[0] = 1;
  }
  sim->rewards[0] = r;
  pack_battle(sim->battle, sim->observations);
  return;
}

#endif