#include "sim.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define DEFAULT_TOTAL_MOVES 1000

static void reset_battle_episode(Battle* battle, TeamConfig cfg) {
  battle->action_queue.q_size = 0;
  battle->turn_num = 0;
  battle->lastMove = NULL;
  battle->lastDamage = 0;
  battle->mode = 0;

  team_generator(&battle->p1, cfg);
  team_generator(&battle->p2, cfg);
}

static int terminal_winner(const Battle* battle) {
  int p1_alive = 0;
  int p2_alive = 0;
  for (int i = 0; i < NUM_POKE; i++) {
    if (!p1_alive && battle->p1.team[i].hp > 0) {
      p1_alive = 1;
    }
    if (!p2_alive && battle->p2.team[i].hp > 0) {
      p2_alive = 1;
    }
    if (p1_alive && p2_alive) {
      return 0;
    }
  }
  if (p1_alive == 0) {
    return 2;
  }
  if (p2_alive == 0) {
    return 1;
  }
  return 0;
}

int main(int argc, char** argv) {
  int total_moves = DEFAULT_TOTAL_MOVES;
  if (argc >= 2) {
    long parsed = strtol(argv[1], NULL, 10);
    if (parsed > 0) {
      total_moves = (int)parsed;
    }
  }

  TeamConfig cfg = GEN_1_OU;
  if (argc >= 3) {
    long parsed_cfg = strtol(argv[2], NULL, 10);
    if (parsed_cfg >= ONE_V_ONE && parsed_cfg < TEAM_CONFIG_MAX) {
      cfg = (TeamConfig)parsed_cfg;
    }
  }

  unsigned int seed = (unsigned int)time(NULL);
  if (argc >= 4) {
    seed = (unsigned int)strtoul(argv[3], NULL, 10);
  }
  sim_srand(seed);

  Battle battle = {0};
  reset_battle_episode(&battle, cfg);

  int moves_simulated = 0;
  int games_completed = 0;

  clock_t start = clock();

  while (moves_simulated < total_moves) {
    int mode = battle.mode;

    int p1_choice = 0;
    int p2_choice = 0;
    if (mode == 0 || mode == 1 || mode == 3) {
      p1_choice = choose_gen1_ai_action(1, &battle.p1, &battle.p2, mode);
    }
    if (mode == 0 || mode == 2 || mode == 3) {
      p2_choice = choose_gen1_ai_action(2, &battle.p2, &battle.p1, mode);
    }

    int next_mode = battle_step(&battle, p1_choice, p2_choice);
    battle.mode = next_mode;

    if (next_mode == 0) {
      battle.mode = end_step(&battle);
    }

    battle.action_queue.q_size = 0;
    moves_simulated++;

    if (battle.mode != 0 && terminal_winner(&battle) != 0) {
      games_completed++;
      reset_battle_episode(&battle, cfg);
    }
  }

  clock_t end = clock();
  double elapsed_sec = (double)(end - start) / (double)CLOCKS_PER_SEC;
  double moves_per_sec = elapsed_sec > 0.0 ? (double)moves_simulated / elapsed_sec : 0.0;

  printf("profile_sim complete\n");
  printf("  seed: %u\n", seed);
  printf("  config: %d\n", (int)cfg);
  printf("  moves: %d\n", moves_simulated);
  printf("  games_completed: %d\n", games_completed);
  printf("  elapsed_sec: %.6f\n", elapsed_sec);
  printf("  moves_per_sec: %.2f\n", moves_per_sec);

  return 0;
}
