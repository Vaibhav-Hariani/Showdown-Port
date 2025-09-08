#ifndef SIM_H
#define SIM_H

#include "battle.h"
#include "battle_queue.h"
#include "move.h"
#include "pokegen.h"
#include "stdio.h"

Battle b = {0};
// ToDo: implement init_battle(), evaluate the queue
// Then, add checks for status effects, misses, etc. etc. Follow OU rules, and
// implement switching mechanics.

// Required struct. Only use floats!
typedef struct {
    float perf; // Recommended 0-1 normalized single real number perf metric
    float score; // Recommended unnormalized single real number perf metric
    float episode_return; // Recommended metric: sum of agent rewards over episode
    float episode_length; // Recommended metric: number of steps of agent episode
    // Any extra fields you add here may be exported to Python in binding.c
    float n; // Required as the last field 
} Log;

// Required that you have some struct for your env
// Recommended that you name it the same as the env file
typedef struct {
    Log log; // Required field. Env binding code uses this to aggregate logs
    unsigned char* observations; // Required. You can use any obs type, but make sure it matches in Python!
    int* actions; // Required. int* for discrete/multidiscrete, float* for box
    float* rewards; // Required
    unsigned char* terminals; // Required. We don't yet have truncations as standard ye
    //The battle object itself
    Battle battle;
    //Unsure what these do
    int size;
    // int tick;
    // int r;
    // int c;
} Sim;

void add_log(Sim* s) {}


//Should not free... Not sure what it should do in this context?
void c_close(Sim* env) {
}
// Required function
void c_reset(Sim* env) {
    memset(env->observations, 0, env->size*env->size*sizeof(unsigned char));
    env->battle = (Battle){0};
    Player p1 = {0};
    Player p2 = {0};
    env->battle.p1 = p1;
    env->battle.p2 = p2;
    team_generator(&env->battle.p1);
    team_generator(&env->battle.p2);
}


// Takes inputs move from both players.
// Resolves queue until choice is required from one (or both) players
// Culls choices that aren't valid
int step(Sim* env, int p1_choice, int p2_choice, int mode) {
  Battle* b = &env->battle;
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
  return eval_queue(b);
  // If this is greater than 0, that means a player has lost a pokemon.
}

void team_generator(Player* p) {
  for(int i = 0; i < 6; i++){
    Pokemon* cur = &p->team[i];
    load_pokemon(cur, NULL, NULL, NULL, NULL, NULL);
  }
  p->active_pokemon.pokemon = &p->team[0];
  p->active_pokemon.type1 = p->active_pokemon.pokemon->type1;
  p->active_pokemon.type2 = p->active_pokemon.pokemon->type2;  
}


int valid_choice(int player_num, Player p, unsigned int input, int mode) {
  // The players input doesn't even matter
  if (mode != player_num && mode != 3 && mode != 0) {
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

// guaranteed to be correct from valid_choice
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

#endif // SIM_H