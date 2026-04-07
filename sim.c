#include "sim.h"

#include <string.h>
#include <time.h>

typedef enum {
  HUMAN_VS_AI = 1,
  HUMAN_VS_HUMAN = 2,
} CliBattleMode;

static int read_int_prompt(const char* prompt, int default_value) {
  char buf[128];
  printf("%s", prompt);
  if (!fgets(buf, sizeof(buf), stdin)) {
    return default_value;
  }
  char* end = NULL;
  long parsed = strtol(buf, &end, 10);
  if (end == buf) {
    return default_value;
  }
  return (int)parsed;
}

static int count_alive(const Player* p) {
  int alive = 0;
  for (int i = 0; i < NUM_POKE; i++) {
    alive += p->team[i].hp > 0 ? 1 : 0;
  }
  return alive;
}

static const char* team_name(TeamConfig cfg) {
  if (cfg == ONE_V_ONE) return "ONE_V_ONE";
  if (cfg == TWO_V_TWO) return "TWO_V_TWO";
  if (cfg == SIX_V_SIX) return "SIX_V_SIX";
  if (cfg == GEN_1_OU) return "GEN_1_OU";
  return "UNKNOWN";
}

static void print_moves_for_active(const Player* p) {
  const BattlePokemon* active = &p->active_pokemon;
  if (!active->pokemon) {
    printf("  No active Pokemon.\n");
    return;
  }
  printf("  Active moves:\n");
  for (int i = 0; i < 4; i++) {
    const Move* m = &active->moves[i];
    if (m->id == NO_MOVE) continue;
    printf("    [%d] %s (PP %d, Pow %d)\n", 6 + i, get_move_name(m->id), m->pp,
           m->power);
  }
}

static void print_player_state(const Player* p, int p_num) {
  printf("\n===== Player %d =====\n", p_num);
  printf("Alive Pokemon: %d\n", count_alive(p));
  for (int i = 0; i < NUM_POKE; i++) {
    const Pokemon* mon = &p->team[i];
    const int active = (i == p->active_pokemon_index);
    if (mon->id == MISSINGNO) continue;
    printf("  [%d] %s%s | HP %d/%d\n",
           i,
           get_pokemon_name(mon->id),
           active ? " (ACTIVE)" : "",
           mon->hp,
           mon->max_hp);
  }
  print_moves_for_active(p);
}

static int prompt_choice(Player* p, int player_num, int mode) {
  int low = 0;
  int high = 9;
  if (mode == player_num || mode == 3) {
    high = 5;
  }

  int choice = -1;
  while (1) {
    if (high == 5) {
      choice = read_int_prompt("Choose switch [0-5]: ", -1);
    } else {
      choice = read_int_prompt("Choose action [0-5 switch, 6-9 move]: ", -1);
    }
    if (choice >= low && choice <= high &&
        valid_choice(player_num, p, (unsigned int)choice, mode)) {
      return choice;
    }
    printf("Invalid choice. Try again.\n");
  }
}

static int ai_best_choice(Sim* sim, int mode) {
  Battle* b = sim->battle;
  if (mode == 2 || mode == 3) {
    return select_valid_switch_choice(&b->p2);
  }

  int choice = gen1_ai_move(&b->p2, &b->p1);
  if (valid_choice(2, &b->p2, (unsigned int)choice, mode)) {
    return choice;
  }

  for (int i = 6; i <= 9; i++) {
    if (valid_choice(2, &b->p2, (unsigned int)i, mode)) {
      return i;
    }
  }
  for (int i = 0; i < NUM_POKE; i++) {
    if (valid_choice(2, &b->p2, (unsigned int)i, mode)) {
      return i;
    }
  }
  return 0;
}

static void init_cli_sim(Sim* sim, TeamConfig cfg, int num_agents) {
  memset(sim, 0, sizeof(*sim));
  sim->battle = (Battle*)calloc(1, sizeof(Battle));
  sim->num_agents = num_agents;
  sim->gametype = cfg;

  int obs_size = PACK_TOTAL_INTS * ((num_agents == 2) ? 2 : 1);
  sim->observations = (int16_t*)calloc((size_t)obs_size, sizeof(int16_t));
  sim->actions = (int*)calloc((size_t)num_agents, sizeof(int));
  sim->rewards = (float*)calloc((size_t)num_agents, sizeof(float));
  sim->terminals = (unsigned char*)calloc((size_t)num_agents, sizeof(unsigned char));

  team_generator(&sim->battle->p1, cfg);
  team_generator(&sim->battle->p2, cfg);
  reset_sim(sim);
  pack_all_agents(sim->battle, sim->num_agents, sim->observations);
}

static void free_cli_sim(Sim* sim) {
  free(sim->observations);
  free(sim->actions);
  free(sim->rewards);
  free(sim->terminals);
  c_close(sim);
}

int main(void) {
  unsigned int seed = (unsigned int)time(NULL);
  sim_srand(seed);

  printf("Showdown-Port CLI\n");
  printf("Seed: %u\n", seed);

  int mode_input = read_int_prompt(
      "Select mode: 1) Human vs AI (GEN1 heuristic) 2) Human vs Human [default 1]: ",
      1);
  CliBattleMode cli_mode =
      (mode_input == HUMAN_VS_HUMAN) ? HUMAN_VS_HUMAN : HUMAN_VS_AI;

  int cfg_input = read_int_prompt(
      "Select team config: 0)1v1 1)2v2 2)6v6 3)GEN1_OU [default 3]: ", 3);
  TeamConfig cfg = GEN_1_OU;
  if (cfg_input >= ONE_V_ONE && cfg_input < TEAM_CONFIG_MAX) {
    cfg = (TeamConfig)cfg_input;
  }

  Sim sim;
  init_cli_sim(&sim, cfg, cli_mode == HUMAN_VS_HUMAN ? 2 : 1);
  Battle* battle = sim.battle;

  printf("\nBattle start: %s\n", team_name(cfg));

  while (1) {
    float terminal_reward = reward(&sim);
    if (terminal_reward == 1.0f || terminal_reward == -1.0f) {
      printf("\nBattle finished. Winner: %s\n",
             terminal_reward > 0.0f ? "Player 1" : "Player 2");
      break;
    }

    printf("\n========== Turn %u | mode=%d =========="
           "\n",
           (unsigned)battle->turn_num + 1,
           battle->mode);
    print_player_state(&battle->p1, 1);
    print_player_state(&battle->p2, 2);

    int p1_choice = 0;
    int p2_choice = 0;

    if (battle->mode == 2) {
      p1_choice = 0;
    } else {
      printf("\nPlayer 1 turn\n");
      p1_choice = prompt_choice(&battle->p1, 1, battle->mode);
    }

    if (battle->mode == 1) {
      p2_choice = 0;
    } else if (cli_mode == HUMAN_VS_HUMAN) {
      printf("\nPlayer 2 turn\n");
      p2_choice = prompt_choice(&battle->p2, 2, battle->mode);
    } else {
      p2_choice = ai_best_choice(&sim, battle->mode);
      printf("AI chose: %d\n", p2_choice);
    }

    battle->mode = battle_step(battle, p1_choice, p2_choice);
    if (battle->mode == 0) {
      battle->mode = end_step(battle);
    }
    battle->action_queue.q_size = 0;
    pack_all_agents(battle, sim.num_agents, sim.observations);
  }

  print_player_state(&battle->p1, 1);
  print_player_state(&battle->p2, 2);
  free_cli_sim(&sim);
  return 0;
}
