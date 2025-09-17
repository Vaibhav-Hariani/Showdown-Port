#ifndef SIM_H
#define SIM_H

#include "sim_utils/battle.h"
#include "sim_utils/battle_queue.h"
#include "sim_utils/move.h"
#include "sim_utils/pokegen.h"
#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"

// Removed globals; memory is now owned per-env inside Sim and allocated on reset

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
  int loser;
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

// return 0 if nobody has won, or player_num(s) if a player has LOST.
int losers(Battle* b) {
  int losers = 0;
  for (int i = 1; i <= 2; i++) {
    int living = 0;
    Player* p = get_player(b, i);
    for (int j = 0; j < 6; j++) {
      if (p->team[j].hp > 0) {
        living = 1;
        break;
      }
    }
    if (living <= 0) {
      losers += i;
    }
  }
  return losers;
}

void team_generator(Player* p) {
  for (int i = 0; i < 6; i++) {
    Pokemon* cur = &p->team[i];
    load_pokemon(cur, NULL, -1);
  }
  p->active_pokemon.pokemon = &p->team[0];
  p->active_pokemon_index = 0;
  p->active_pokemon.type1 = p->active_pokemon.pokemon->type1;
  p->active_pokemon.type2 = p->active_pokemon.pokemon->type2;
}

int internal_step(Sim* sim) {
  int p1_choice = sim->actions[0];
  int p2_choice = rand() % 10;
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
    action(b, &b->p1, &b->p2, p1_choice, REGULAR);
    action(b, &b->p2, &b->p1, p2_choice, REGULAR);

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
  int a = losers(b);
  return a ? 10 + a : mode; 
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
  packed |= (int16_t)(move->id & 0xFF) << 0;    // 8 bits for move ID (0-255)
  packed |= (int16_t)(move->pp & 0x3F) << 8;    // 6 bits for PP (0-63)
  return packed;
}

// Packs all pokemon in the battle into the provided int array.
// Each pokemon: [id, hp, status_flags, (stat_mods if active), move1, move2, move3, move4]
// Returns the number of ints written.
int16_t pack_status(Pokemon* p) {
  int16_t packed = 0;
  packed |= (p->status.paralyzed & 0x1) << 0;
  packed |= (p->status.burn & 0x1) << 1;
  packed |= (p->status.freeze & 0x1) << 2;
  packed |= (p->status.poison & 0x1) << 3;
  packed |= ((p->status.sleep > 0) & 0x1) << 4;  // 3 bits for sleep counter, just checking if it's greater than zero
  return packed;
}

void pack_battle(Battle* b, int16_t* out) {
  // Each pokemon: [id, hp, status_flags, (stat_mods if active), move1, move2, move3, move4]
  // 6 pokemon per player, 2 players
  // Active pokemon have 2 extra ints for stat mods
  // Flattened array: 12 rows * 9 columns = 108 elements total
  for (int i = 0; i < 2; i++) {
    Player* p = get_player(b, i + 1);
    for (int j = 0; j < 6; j++) {
      Pokemon* cur = &p->team[j];
      int pokemon_index = i * 6 + j;        // Pokemon index (0-11)
      int base_offset = pokemon_index * 9;  // Each pokemon takes 9 slots
      int16_t* row = out + base_offset;
      // Pack basic pokemon info
      row[0] = cur->id;
      row[1] = cur->hp;
      row[2] = pack_status(cur);
      if (j == p->active_pokemon_index) {
        row[0] *= -1;  // Mark active pokemon with negative id
        stat_mods* mods = &p->active_pokemon.stat_mods;
        row[2] |= ((p->active_pokemon.confusion_counter > 0) & 0x1) << 5;  // Add confusion bit for active pokemon
        row[3] = pack_attack_def_specA_specD(mods);
        row[4] = pack_stat_acc_eva(mods);
      } else {
        // Non-active pokemon don't have stat mods
        row[3] = 0;
        row[4] = 0;
      }
      // Pack move data for all 4 moves
      for (int k = 0; k < 4; k++) {
        row[5 + k] = pack_move(&cur->poke_moves[k]);
      }
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

void c_render(Sim* sim) { 
  pack_battle(sim->battle, sim->observations);
  if (sim->battle->mode >= 10) {
    sim->loser = sim->battle->mode - 10;
  }
}

void c_close(Sim* s) {
  if (s->battle) {
    free(s->battle); // Frees the entire slab (Battle + Teams + Moves)
    s->battle = NULL;
  }
  return;
}
void c_step(Sim* sim) {
  int a = internal_step(sim);
  sim->battle->mode = a;
  if (a == 0) {
    sim->battle->mode = end_step(sim->battle);
  }
  //No end step if a pokemon has fainted (gen1 quirk). Simply clear the queue and move on
  sim->battle->action_queue.q_size = 0; // Clear the queue before accepting switches
  sim->tick++;
  return;
}

#endif