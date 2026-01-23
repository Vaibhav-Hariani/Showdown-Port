#ifndef SIM_LOGGING_H
#define SIM_LOGGING_H

#include "../data_sim/typing.h"
#include "battle.h"

// Log struct definition (if not already defined)

typedef struct {
  float num_moves;
  float num_won;
  float num_lost;
  float percent_valid_moves;
  float opponent_final_hp;
  float perf;
  float score;
  float episode_return;
  float episode_length;
  float n;
  // Game-specific counters (internal)
  float gen1_wins;
  float six_wins;
} Log;


typedef enum TeamConfig {
  ONE_V_ONE = 0,
  TWO_V_TWO,
  SIX_V_SIX,
  GEN_1_OU,
  TEAM_CONFIG_MAX
} TeamConfig;

// Log everything in a single function
void log_episode(Log* log,
                 Battle* b,
                 int reward,
                 int valid_moves,
                 int invalid_moves,
                 int ticks,
                 TeamConfig gametype) {
  log->n += 1.0f;
  log->num_moves += ticks;
  log->episode_length += ticks;
  log->percent_valid_moves += (float)valid_moves / (float)ticks;

  log->episode_return += (reward + 0.01 * invalid_moves);

  if (reward == 1.0f) {
    log->num_won += 1.0f;
    log->perf += 1.0f;
    if (gametype == SIX_V_SIX) {
      log->six_wins += 2.0f;
    } else if (gametype == GEN_1_OU) {
      log->gen1_wins += 2.0f;
    }
  } else if (reward == -1.0f) {
    float opp_hp = 0.0;
    for (int i = 0; i < NUM_POKE; i++) {
      opp_hp += b->p2.team[i].hp;
    }
    log->opponent_final_hp += opp_hp;
    log->num_lost += 1.0f;
  }
  
  return;
}
#endif