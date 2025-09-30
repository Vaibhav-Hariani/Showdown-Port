#ifndef SIM_H
#define SIM_H

#include "sim_utils/battle.h"
#include "sim_utils/battle_queue.h"
#include "sim_utils/move.h"
#include "sim_utils/pokegen.h"
#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"

// Removed globals; memory is now owned per-env inside Sim and allocated on
// reset

typedef struct {
  // n is num_games
  // int num_games;
  // Ticks is a local num_moves
  float num_moves;
  float num_won;
  float num_lost;
  float invalid_moves;
  float valid_moves;

  float win_rate;
  float avg_game_len;
  float avg_win_len;
  float avg_damage_pct;
  
  // Squared metrics
  float perf;           // 0-1 normalized performance metric
  float score;          // Unnormalized score metric
  float episode_return; // Sum of agent rewards over episode
  float episode_length; // Number of steps of agent episode
  
  float n;  // Required as the last field
} Log;

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

// // Triggered when a game ends
// void sub_log_update(Log* log, Sim* s) {
//   log->num_moves += s->tick;

// }

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

void final_update(Log* log, Sim* s) {
  // Updating avg: (n * prev + new) / (n + 1)
  //  (n + 1 * prev) + (new - prev) / (n + 1)
  //  avg += (new - prev) / (n + 1)
  if (s->rewards[0] > 0) {
    log->num_won += 1.0f;
    log->avg_win_len += (s->tick - log->avg_win_len) / log->num_won;
    // For perf, treat win as 1.0 (perfect performance)
    log->perf = 1.0f;  // Set to 1.0 for win regardless of accumulated perf
  } else {
    log->num_lost += 1;
  }
  log->n += 1.0f;
}

// Returns a reward in [-1, 1]:
// -1 if player 1 has lost all Pokémon
//  1 if player 2 has lost all Pokémon
// Otherwise, mean percent HP remaining for both teams, normalized: (mean_p1 -
// mean_p2)
float reward(Sim* s) {
  Battle* b = s->battle;
  float p1_percent_sum = 0, p2_percent_sum = 0;
  int num_p1 = 0, num_p2 = 0;
  for (int j = 0; j < 6; j++) {
    if (b->p1.team[j].max_hp > 0) {
      num_p1++;
      float hp1 = b->p1.team[j].hp;
      p1_percent_sum += hp1 / b->p1.team[j].max_hp;
    }
    if (b->p2.team[j].max_hp > 0) {
      num_p2++;
      float hp2 = b->p2.team[j].hp;
      p2_percent_sum += hp2 / b->p2.team[j].max_hp;
    }
  }
  // Todo: these calculations should observe the number of pokemon on a players
  // team
  float mean_p1 = p1_percent_sum / num_p1;
  float mean_p2 = p2_percent_sum / num_p2;
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
  // Disabling anything other than pokemon 1... must be easy!
  load_pokemon(&p->team[0], NULL, 0);
  for (int i = 1; i < 6; i++) {
    Pokemon* cur = &p->team[i];
    memset(cur, 0, sizeof(Pokemon));
    // load_pokemon(cur, NULL, 0);
  }
  p->active_pokemon.pokemon = &p->team[0];
  p->active_pokemon_index = 0;
  p->active_pokemon.type1 = p->active_pokemon.pokemon->type1;
  p->active_pokemon.type2 = p->active_pokemon.pokemon->type2;
}

static inline int battle_step(Sim* sim, int choice) {
  int p1_choice = choice;

  Battle* b = sim->battle;
  // 5 year old: choose the move that hits the hardest
  int p2_choice = get_highest_damage_move_index(&b->p2);
  // Make a regular move if feasible
  int mode = b->mode;
  while (!valid_choice(2, b->p2, p2_choice, mode)) {
    p2_choice = rand() % 10;
  }

  if (!valid_choice(1, b->p1, p1_choice, mode)) {
    log_mini(&sim->log, -1, -1.0f); // Track invalid move via log_mini with penalty
    return -1;
  }
  // Should never arrive here
  if (!valid_choice(2, b->p2, p2_choice, mode)) {
    return -2;
  }
  // Track valid move (with 0.0f reward for now - actual reward calculated later)
  log_mini(&sim->log, 1, 0.0f);
  if (mode == 0) {
    if ((!b->p1.active_pokemon.pokemon->status.freeze &&
         !b->p1.active_pokemon.pokemon->status.sleep) ||
        p1_choice < 6) {
      action(b, &b->p1, &b->p2, p1_choice, REGULAR);
    }
    if ((!b->p2.active_pokemon.pokemon->status.freeze &&
         !b->p2.active_pokemon.pokemon->status.sleep) ||
        p2_choice < 6) {
      action(b, &b->p2, &b->p1, p2_choice, REGULAR);
    }

  } else {
    // player 1 has lost a pokemon
    if ((mode == 1 || mode == 3)) {
      action(b, &b->p1, &b->p2, p1_choice, FAINTED);
    } else {
      action(b, &b->p2, &b->p1, p2_choice, FAINTED);
    }
  }
  // Sort & evaluate the battlequeue on a move by move basis
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
    // sim->log = (Log){0};
  } else {
    clear_battle(sim->battle);
  }
  sim->tick = 0;
  sim->rewards[0] = 0.0f;
  
  // Reset episode-specific metrics for new episode
  sim->log.episode_return = 0.0f;
  sim->log.episode_length = 0.0f;
  sim->log.perf = 0.0f;  // Reset performance metric too
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
    update_log(&sim->log, sim);
    c_reset(sim);
    // Set terminal flag so that model knows to reload embeddings + to prevent model self-burn
    sim->terminals[0] = 1;
  }
  sim->rewards[0] = r;
  pack_battle(sim->battle, sim->observations);
  return;
}

int16_t pack_attack_def_specA_specD(stat_mods* mods) {
  int16_t packed = 0;
  packed |= (mods->attack & 0xF) << 0;
  packed |= (mods->defense & 0xF) << 4;
  packed |= (mods->specA & 0xF) << 8;
  packed |= (mods->specD & 0xF) << 12;
  return packed;
}

int16_t pack_stat_acc_eva(stat_mods* mods) {
  int16_t packed = 0;
  packed |= (mods->speed & 0xF) << 0;
  packed |= (mods->accuracy & 0xF) << 4;
  packed |= (mods->evasion & 0xF) << 8;
  return packed;
}
// Packs move data for a single move into an int16_t
// Format: [move_id(8 bits), pp(6 bits)] - 2 bits unused
int16_t pack_move(Move* move) {
  int16_t packed = 0;
  packed |= (int16_t)(move->id & 0xFF) << 0;  // 8 bits for move ID (0-255)
  packed |= (int16_t)(move->pp & 0x3F) << 8;  // 6 bits for PP (0-63)
  return packed;
}

// Packs all pokemon in the battle into the provided int array.
// Each pokemon: [id, hp, status_flags, (stat_mods if active), move1, move2,
// move3, move4] Returns the number of ints written.
int16_t pack_status(Pokemon* p) {
  int16_t packed = 0;
  packed |= (p->status.paralyzed & 0x1) << 0;
  packed |= (p->status.burn & 0x1) << 1;
  packed |= (p->status.freeze & 0x1) << 2;
  packed |= (p->status.poison & 0x1) << 3;
  packed |= (p->status.sleep & 0x1) << 4;

  return packed;
}

void pack_poke(int16_t* row, Player* player, int poke_index) {
  Pokemon* poke = &player->team[poke_index];

  // Pack pokemon number first
  row[0] = poke->id;

  // Pack move data next (positions 1-4)
  for (int k = 0; k < 4; k++) {
    row[1 + k] = pack_move(&poke->poke_moves[k]);
  }

  // Pack everything else after move data
  row[5] = poke->hp;
  // Also contains confusion if the pokemon is active (and confused)
  row[6] = pack_status(poke);

  if (poke_index == player->active_pokemon_index) {
    row[0] *= -1;  // Mark active pokemon with negative id
    stat_mods* mods = &player->active_pokemon.stat_mods;
    row[7] = pack_attack_def_specA_specD(mods);
    row[8] = pack_stat_acc_eva(mods);
  } else {
    row[7] = 0;
    row[8] = 0;
  }
}

void pack_battle(Battle* b, int16_t* out) {
  // Each pokemon: [id, move1, move2, move3, move4, hp, status_flags, stat_mod1,
  // stat_mod2] 6 pokemon per player, 2 players Active pokemon have 2 extra ints
  // for stat mods Flattened array: 12 rows * 9 columns = 108 elements total
  for (int i = 0; i < 2; i++) {
    Player* p = get_player(b, i + 1);
    for (int j = 0; j < 6; j++) {
      int pokemon_index = i * 6 + j;
      int base_offset = pokemon_index * 9;
      int16_t* row = out + base_offset;
      // Forcing the obscuring of unseen opponent pokemon
      if (i == 2 && !(b->p1.shown_pokemon & (1 << j))) {
        for (int z = 0; z < 9; z++) row[z] = 0;
        continue;
      }
      pack_poke(row, p, j);
    }
  }
}

#endif