#ifndef SIM_LOGGING_H
#define SIM_LOGGING_H

#include "data_sim/typing.h"
#include "sim_utils/battle.h"

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

// Log everything in a single function
void log_episode(Log* log, Battle* b, int reward, int valid_moves, int invalid_moves, int ticks, int gametype) {
  
  log->n += 1.0f;
  log->num_moves += ticks;
  log->episode_length += ticks;
  log->percent_valid_moves += (float) valid_moves / (float) ticks;
  
  log->episode_return += (reward + 0.01 * invalid_moves);
  // // Increment gametype-specific counters for visualization
  if (gametype == 0) {
      // log->six_games += 1;
      //Approximate scaling as every other game is going to be a six game: this is a mini hack
      if (reward == 1.0f) {log->six_wins += 2.0f;}
  } else if (gametype == 1) {
      // log->gen1_games += 1;
      if (reward == 1.0f) {log->gen1_wins += 2.0f;}
  }
  if (reward == 1.0f) {
    log->num_won += 1.0f;
    log->perf += 1.0f;
  } else if (reward == -1.0f) {
    float opp_hp = 0.0;
    for(int i = 0; i < NUM_POKE; i++) {
      opp_hp += b->p2.team[i].hp;
    }
    log->opponent_final_hp += opp_hp;
    log->num_lost += 1.0f;
  }
  
  return;
}
#endif

//Keeping just in case

// void initial_log(Log* log, Battle* battle);
// void final_update(Log* log,
//                   float reward,
//                   float mean_p1_hp,
//                   float mean_p2_hp,
//                   float avg_damage_pct,
//                   int tick,
//                   int episode_valid_moves,
//                   int episode_invalid_moves);


// // Initial logging function - combines team HP logging and matchup scoring
// // Called at the beginning of each game
// void initial_log(Log* log, Battle* battle) {
//   // Log opponent team average HP
//   float total_hp = 0.0f;
//   Player* p2 = &battle->p2;
//   for (int i = 0; i < NUM_POKE; i++) {
//     Pokemon* p2_poke_hp = &p2->team[i];
//     total_hp += p2_poke_hp->hp;
//     // Count damaging moves (power > 0) for P2 here
//     // for (int m = 0; m < 4; m++) {
//     //   if (p2_poke_hp->poke_moves[m].power > 0) team_damaging_p2++;
//     // }
//   }
//   log->opponent_avg_hp += total_hp / NUM_POKE;

//   // Calculate type matchup advantage and team strength
//   float score = 0.0f;
//   float avg_poke_power = 0;
//   // For each Pokemon on Player 1's team (all NUM_POKE Pokemon are valid)
//   for (int p1_idx = 0; p1_idx < NUM_POKE; p1_idx++) {
//     Pokemon* p1_poke = &battle->p1.team[p1_idx];
//     // Calculate move power statistics
//     for (int i = 0; i < 4; i++) {
//       int power = p1_poke->poke_moves[i].power;
//       avg_poke_power += power;
//     }
//     // Check this Pokemon against all of Player 2's Pokemon for matchup scoring
//     for (int p2_idx = 0; p2_idx < NUM_POKE; p2_idx++) {
//       Pokemon* p2_poke = &battle->p2.team[p2_idx];

//       // Calculate P1 Pokemon's best type effectiveness against P2 Pokemon
//       float type1_eff = damage_chart[p1_poke->type1][p2_poke->type1] *
//                         damage_chart[p1_poke->type1][p2_poke->type2];
//       float type2_eff = damage_chart[p1_poke->type2][p2_poke->type1] *
//                         damage_chart[p1_poke->type2][p2_poke->type2];
//       float p1_effectiveness = (type2_eff > type1_eff) ? type2_eff : type1_eff;

//       // Calculate P2 Pokemon's best type effectiveness against P1 Pokemon
//       type1_eff = damage_chart[p2_poke->type1][p1_poke->type1] *
//                   damage_chart[p2_poke->type1][p1_poke->type2];
//       type2_eff = damage_chart[p2_poke->type2][p1_poke->type1] *
//                   damage_chart[p2_poke->type2][p1_poke->type2];
//       float p2_effectiveness = (type2_eff > type1_eff) ? type2_eff : type1_eff;

//       // Compare effectiveness and award points
//       if (p1_effectiveness > p2_effectiveness) {
//         score += 1.0f;  // P1 has advantage
//       } else if (p2_effectiveness > p1_effectiveness) {
//         score -= 1.0f;  // P2 has advantage
//       }
//       // If equal, no change to score
//     }
//   }

//   // Update log with calculated values
//   avg_poke_power /= (float)NUM_POKE * 4;
//   log->team_strength += avg_poke_power;
//   log->matchup_advantage += score;
//   // Damaging move metrics disabled
// }

// void final_update(Log* log,
//                   float reward,
//                   float mean_p1_hp,
//                   float mean_p2_hp,
//                   float avg_damage_pct,
//                   int tick,
//                   int episode_valid_moves,
//                   int episode_invalid_moves) {
//   // num_moves is now tracked incrementally each step; do not add tick here to
//   // avoid double counting
//   log->episode_length += (float)tick;
//   // Accumulate total moves for this episode as tick count (each tick
//   // corresponds to a move decision)
//   log->num_moves += (float)tick;
//   // Track wins and losses (will be summed, then averaged by n to get win/loss
//   // rate)
//   log->episode_return += reward;

//   if (reward > 0) {
//     log->num_won += 1.0f;
//     log->perf += 1.0f;
//   } else {
//     log->num_lost += 1.0f;
//   }
//   // Record final HP values at end of episode (will be summed, then averaged by
//   // n)
//   log->mean_p1_hp += mean_p1_hp;
//   log->mean_p2_hp += mean_p2_hp;
//   log->avg_damage_pct += avg_damage_pct;
//   // Compute per-episode percent valid moves and accumulate
//   int mv_total = episode_valid_moves + episode_invalid_moves;
//   log->percent_valid_moves += (float)episode_valid_moves / (float)mv_total;
//   // Increment episode counter (used for averaging all metrics in vec_log)
//   log->n += 1.0f;
// }
