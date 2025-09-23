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

// Not modifying or using for right now: leaving for future use
typedef struct {
  float perf;   // Recommended 0-1 normalized single real number perf metric
  float score;  // Recommended unnormalized single real number perf metric
  float
      episode_return;  // Recommended metric: sum of agent rewards over episode
  float episode_length;  // Recommended metric: number of steps of agent episode
  // Any extra fields you add here may be exported to Python in binding.c
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
  // Not strictly necessary,
  //  figure this might make life a bit easier with de-rewarding long running
  //  games.
  int tick;
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
  return;
}

// Returns a reward in [-1, 1]:
// -1 if player 1 has lost all Pokémon
//  1 if player 2 has lost all Pokémon
// Otherwise, mean percent HP remaining for both teams, normalized: (mean_p1 -
// mean_p2)
float reward(Battle* b) {
  float p1_percent_sum = 0, p2_percent_sum = 0;
  for (int j = 0; j < 6; j++) {
    float hp1 = b->p1.team[j].hp;
    p1_percent_sum += hp1 > 0 ? (hp1 / b->p1.team[j].max_hp) : 0.0f;
    float hp2 = b->p2.team[j].hp;
    p2_percent_sum += hp2 > 0 ? (hp2 / b->p2.team[j].max_hp) : 0.0f;
  }
  float mean_p1 = p1_percent_sum / 6.0f;
  float mean_p2 = p2_percent_sum / 6.0f;

  // These checks are likely unnecessary...
  if (p1_percent_sum == 0.0f) return -1.0f;
  if (p2_percent_sum == 0.0f) return 1.0f;
  float result = mean_p1 - mean_p2;
  // Clamp to [-1, 1] just in case
  if (result > 1.0f) result = 1.0f;
  if (result < -1.0f) result = -1.0f;
  return result;
}

void team_generator(Player* p) {
  //Disabling anything other than pokemon 1... must be easy!
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

int internal_step(Sim* sim, int choice) {
  int p1_choice = choice;

  // Make a regular move if feasible
  // Make a regular move if feasible
  int p2_choice = rand() % 4 + 6;
  Battle* b = sim->battle;
  int mode = b->mode;

  while (!valid_choice(2, b->p2, p2_choice, mode)) {
    p2_choice = rand() % 10;
  }
  if (mode == 0) {
    if (!(valid_choice(1, b->p1, p1_choice, mode) &&
          valid_choice(2, b->p2, p2_choice, mode))) {
      return -1;
    }
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
      if (!valid_choice(1, b->p1, p1_choice, mode)) {
        return -1;
      }
      action(b, &b->p1, &b->p2, p1_choice, FAINTED);
    }
    if ((mode == 2 || mode == 3)) {
      if (!valid_choice(2, b->p2, p2_choice, mode)) {
        return -1;
      }
      action(b, &b->p2, &b->p1, p2_choice, FAINTED);
    }
  }
  // Sort & evaluate the battlequeue on a move by move basis
  mode = eval_queue(b);
  b->mode = mode;
  float a = reward(b);
  sim->rewards[0] = a;
  if (a == 1.0f || a == -1.0f) {
    sim->terminals[0] == 1;  // Game over
  }
  return mode;
  // If this is greater than 0, that means a player has lost a pokemon. If it is
  // 10, the game is
}

// For generating the observation space.
//  Helper: Packs 6 stat mods (each 4 bits) into a single int (24 bits used)
//  Order: Atk (0-3), Def (4-7), Spd (8-11), SpecA (12-15), Acc (16-19), Eva
//  (20-23)
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
  row[0] = poke->id +
          
           1;   // Currently starting pokedex at zero; this should fix that (?)
  row[1] = poke->hp;
  // Also contains confusion if the pokemon is active (and confused)
  row[2] = pack_status(poke);
  for (int k = 0; k < 4; k++) {
    row[3 + k] = pack_move(&poke->poke_moves[k]);
  }
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
  // Each pokemon: [id, hp, status_flags, (stat_mods if active), move1, move2,
  // move3, move4] 6 pokemon per player, 2 players Active pokemon have 2 extra
  // ints for stat mods Flattened array: 12 rows * 9 columns = 108 elements
  // total
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
  sim->tick = 0;
  // Allocate Battle on first use; Player.team and Pokemon.poke_moves are inline
  if (sim->battle == NULL) {
    sim->battle = (Battle*)calloc(1, sizeof(Battle));
  }

  clear_battle(sim->battle);
  team_generator(&sim->battle->p1);
  team_generator(&sim->battle->p2);
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
  int a = internal_step(sim, sim->actions[0]);
  sim->battle->mode = a;
  if (a == 0) {
    sim->battle->mode = end_step(sim->battle);
  }
  // No end step if a pokemon has fainted (gen1 quirk). Simply clear the queue
  // and move on
  sim->battle->action_queue.q_size = 0;
  sim->battle->action_queue.q_size = 0;
  sim->tick++;
  pack_battle(sim->battle, sim->observations);
  return;
}

#endif