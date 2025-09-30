#ifndef SIM_LOGGING_H
#define SIM_LOGGING_H

#include "sim_utils/battle.h"
#include "data_sim/typing.h"

// Log struct definition (if not already defined)

typedef struct {
  float num_moves;
  float num_won;
  float num_lost;
  float invalid_moves;
  float valid_moves;
  float win_rate;
  float avg_damage_pct;
  float team_strength; 
  float matchup_advantage;
  float damage_dealing_move_pct;
  float max_damage_move;
  float highest_damage_value;
  float opponent_avg_hp;
  float perf;
  float score;
  float episode_return;
  float episode_length;
  float n;
} Log;

// Logging function declarations
void initial_log(Log* log, Battle* battle);
void log_mini(Log* log, int valid_move, float reward);
void final_update(Log* log, float reward);

// Implementation
// Initial logging function - combines team HP logging and matchup scoring
// Called at the beginning of each game
void initial_log(Log* log, Battle* battle) {
  // Log opponent team average HP
  float total_hp = 0.0f;
  Player* p2 = &battle->p2;
  for (int i = 0; i < NUM_POKE; i++) {
    total_hp += p2->team[i].hp;
  }
  log->opponent_avg_hp += total_hp / NUM_POKE;

  // Calculate type matchup advantage and team strength
  float score = 0.0f;
  int avg_poke_power = 0;
  int max_move_pow = 0;
  float highest_damage_value = 0.0f;
  
  // For each Pokemon on Player 1's team (all NUM_POKE Pokemon are valid)
  for (int p1_idx = 0; p1_idx < NUM_POKE; p1_idx++) {
    Pokemon* p1_poke = &battle->p1.team[p1_idx];
    
    // Calculate move power statistics
    for(int i = 0; i < 4; i++){
      int power = p1_poke->poke_moves[i].power;
      avg_poke_power += power;
      max_move_pow = (power > max_move_pow) ? power : max_move_pow;
      
      // Track the highest damage move
      if (power > highest_damage_value) {
        highest_damage_value = (float)power;
      }
    }
    
    // Check this Pokemon against all of Player 2's Pokemon for matchup scoring
    for (int p2_idx = 0; p2_idx < NUM_POKE; p2_idx++) {
      Pokemon* p2_poke = &battle->p2.team[p2_idx];
      
      // Calculate P1 Pokemon's best type effectiveness against P2 Pokemon
      float type1_eff = damage_chart[p1_poke->type1][p2_poke->type1] * 
                       damage_chart[p1_poke->type1][p2_poke->type2];
      float type2_eff = damage_chart[p1_poke->type2][p2_poke->type1] * 
                       damage_chart[p1_poke->type2][p2_poke->type2];
      float p1_effectiveness = (type2_eff > type1_eff) ? type2_eff : type1_eff;
      
      // Calculate P2 Pokemon's best type effectiveness against P1 Pokemon
      type1_eff = damage_chart[p2_poke->type1][p1_poke->type1] * 
                  damage_chart[p2_poke->type1][p1_poke->type2];
      type2_eff = damage_chart[p2_poke->type2][p1_poke->type1] * 
                  damage_chart[p2_poke->type2][p1_poke->type2];
      float p2_effectiveness = (type2_eff > type1_eff) ? type2_eff : type1_eff;
      
      // Compare effectiveness and award points
      if (p1_effectiveness > p2_effectiveness) {
        score += 1.0f; // P1 has advantage
      } else if (p2_effectiveness > p1_effectiveness) {
        score -= 1.0f; // P2 has advantage
      }
      // If equal, no change to score
    }
  }
  
  // Update log with calculated values
  avg_poke_power /= (NUM_POKE * 4); // Average over all moves of all Pokemon slots
  log->team_strength += avg_poke_power;
  log->matchup_advantage += score;
  log->highest_damage_value += highest_damage_value;
}

void log_mini(Log* log, int valid_move, float reward) {
  log->num_moves += 1.0f;
  if (valid_move > 0) {
    log->valid_moves += 1.0f;
  } else if (valid_move < 0) {
    log->invalid_moves += 1.0f;
  }
  // Update episode length for squared metrics
  log->episode_length += 1.0f;
  
  // Track rewards for score and episode_return
  log->episode_return += reward;
  log->score += reward;

  // Update perf metric based on reward
  // For positive rewards, consider it as performance contribution
  if (reward > 0.0f) {
    log->perf += reward; // Only add positive rewards to perf
  }
}

void final_update(Log* log, float reward) {
  // Updating avg: (n * prev + new) / (n + 1)
  //  (n + 1 * prev) + (new - prev) / (n + 1)
  //  avg += (new - prev) / (n + 1)
  if (reward > 0) {
    log->num_won += 1.0f;
    // For perf, treat win as 1.0 (perfect performance)
    log->perf = 1.0f;  // Set to 1.0 for win regardless of accumulated perf
  } else {
    log->num_lost += 1;
  }
  log->n += 1.0f;
}

#endif