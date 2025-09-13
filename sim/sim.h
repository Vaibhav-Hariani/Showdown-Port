#ifndef SIM_H
#define SIM_H

#include "battle.h"
#include "battle_queue.h"
#include "move.h"
#include "pokegen.h"
#include "stdint.h"
#include "stdio.h"

Battle b = {0};

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
  uint16_t** observations;  // Required. You can use any obs type, but make sure
                            // it matches in Python!
  int* actions;    // Required. int* for discrete/multidiscrete, float* for box
  float* rewards;  // Required
  unsigned char*
      terminals;  // Required. We don't yet have truncations as standard yet
  int size;
  Battle* battle;
  int mode;  // input mode switcher
} Sim;

// Convert to obs of ints. We can do player1 followed by p2, array of length 2*6
// * (4 (moves), every status effect, hp,
void print_state(Player* player) {
  for (int i = 0; i < 6; i++) {
    Pokemon p = player->team[i];
    if (player->active_pokemon_index == i) {
      printf("Active Pokemon: #%d: %s \n", i, get_pokemon_name(p.id));
      printf(
          "Stat Modifiers: Atk=%d, Def=%d, Spd=%d, Spec=%d, Acc=%d, "
          "Eva=%d \n",
          player->active_pokemon.stat_mods.attack,
          player->active_pokemon.stat_mods.defense,
          player->active_pokemon.stat_mods.speed,
          player->active_pokemon.stat_mods.specA,
          player->active_pokemon.stat_mods.accuracy,
          player->active_pokemon.stat_mods.evasion);
    } else {
      printf("Pokemon #%d: %s \n", i, get_pokemon_name(p.id));
    }
    printf("HP: %d \t", p.hp);
    printf("Level: %d \t", p.stats.level);
    printf("Types: %d, %d \t", p.type1, p.type2);
    printf("Available Moves: \n");
    for (int j = 0; j < 4; j++) {
      Move m = p.poke_moves[j];
      printf(
          "\t Label: %s; PP Remaining: %d; Power: %d; Accuracy: %.2f; Type: "
          "%d; Category: %d \n",
          get_move_name(m.id),
          m.pp,
          m.power,
          m.accuracy,
          m.type,
          m.category);
    }
  }
}

int valid_choice(int player_num, Player p, unsigned int input, int mode) {
  // The players input doesn't even matter
  if (!(mode == player_num || mode == 3 || mode == 0)) {
    return 1;
  }
  if (input <= 6) {
    return valid_switch(p, input);
  }
  if (mode == 0 && input <= 9) {
    return valid_move(&p, input - 6);
  }
  return 0;
}
void action(Battle* b, Player* user, Player* target, int input, int type) {
  // Action* cur = (b->action_queue.queue) + b->action_queue.q_size;
  if (input >= 7) {
    input -= 7;
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
    load_pokemon(cur, NULL, NULL, NULL, NULL, NULL);
  }
  p->active_pokemon.pokemon = &p->team[0];
  p->active_pokemon.type1 = p->active_pokemon.pokemon->type1;
  p->active_pokemon.type2 = p->active_pokemon.pokemon->type2;
}

int step(Sim* sim) {
  int mode = sim->mode;
  int p1_choice = sim->actions[0];
  int p2_choice = rand() % 10;
  Battle* b = sim->battle;

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
  b->mode = mode;
  int a = losers(b);
  if (a) {
    b->mode = 10 + a;
    return 10 + a;
  }
  return mode;
  // If this is greater than 0, that means a player has lost a pokemon. If it is
  // 10, the game is
}

// For generating the observation space.
//  Helper: Packs 6 stat mods (each 4 bits) into a single int (24 bits used)
//  Order: Atk (0-3), Def (4-7), Spd (8-11), SpecA (12-15), Acc (16-19), Eva
//  (20-23)
int pack_attack_def_specA_specD(stat_mods* mods) {
  int packed = 0;
  packed |= (mods->attack & 0xF) << 0;
  packed |= (mods->defense & 0xF) << 4;
  packed |= (mods->specA & 0xF) << 8;
  packed |= (mods->specD & 0xF) << 12;
  return packed;
}

int pack_stat_acc_eva(stat_mods* mods) {
  int packed = 0;
  packed |= (mods->speed & 0xF) << 0;
  packed |= (mods->accuracy & 0xF) << 4;
  packed |= (mods->evasion & 0xF) << 8;
  return packed;
}
// Packs all pokemon in the battle into the provided int array.
// Each pokemon: [id, hp, status_flags, (stat_mods if active)]
// Returns the number of ints written.
int pack_status(Pokemon* p) {
  int packed = 0;
  packed |= (p->status.paralyzed & 0x1) << 0;
  packed |= (p->status.burn & 0x1) << 1;
  packed |= (p->status.freeze & 0x1) << 2;
  packed |= (p->status.poison & 0x1) << 3;
  packed |= ((p->status.sleep > 0) & 0x1) << 4;  // 3 bits for sleep counter
  return packed;
}

void pack_gen_poke(uint16_t* out, Pokemon* p) {
  out[0] = p->id;
  out[1] = p->hp;
  out[2] = pack_status(p);
}

void pack_battle(Battle* b, uint16_t** out) {
  // Each pokemon: [id, hp, status_flags, (stat_mods if active)]
  // 6 pokemon per player, 2 players
  // Active pokemon have 2 extra ints for stat mods
  for (int i = 0; i < 2; i++) {
    Player* p = get_player(b, i + 1);
    for (int j = 0; j < 6; j++) {
      Pokemon* cur = &p->team[j];
      uint16_t* row = out[i * 6 + j];
      pack_gen_poke(row, cur);
      if (j == p->active_pokemon_index) {
        row[0] *= -1;  // Mark active pokemon with negative id
        stat_mods* mods = &p->active_pokemon.stat_mods;
        row[3] = pack_attack_def_specA_specD(mods);
        row[4] = pack_stat_acc_eva(mods);
      }
    }
  }
}

void c_render(Sim* sim) { pack_battle(sim->battle, sim->observations); }
void c_close(Sim* s) { return; }
void c_step(Sim* sim) { step(sim);}

#endif