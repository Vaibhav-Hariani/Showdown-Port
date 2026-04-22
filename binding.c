#include "sim.h"

#define Env Sim
#include "../env_binding.h"

static int my_init(Env* env, PyObject* args, PyObject* kwargs) {
  (void)args;
  int num_agents = unpack(kwargs, "num_agents");
  if (num_agents < 1 || num_agents > 2) {
    num_agents = 1;
  }
  env->num_agents = num_agents;

  int gametype_limit = unpack(kwargs, "gametype_limit");
  if (gametype_limit <= 0) {
    gametype_limit = unpack(kwargs, "max_pokemon");
  }
  if (gametype_limit <= 0) {
    gametype_limit = unpack(kwargs, "max_gametype");
  }
  if (gametype_limit < 1 || gametype_limit > TEAM_CONFIG_MAX) {
    gametype_limit = TEAM_CONFIG_MAX;
  }
  env->max_gametype = gametype_limit;

  int opp_type = unpack(kwargs, "opp_type");
  if (opp_type < 1 || opp_type > 3) {
    opp_type = 3;
  }
  env->opp_type = opp_type;

  sim_init(env);
  return 0;
}

static int my_log(PyObject* dict, Log* log) {
  // Original Showdown metrics
  assign_to_dict(dict, "num_moves", log->num_moves);
  assign_to_dict(dict, "num_won", log->num_won);
  assign_to_dict(dict, "num_lost", log->num_lost);
  assign_to_dict(dict, "percent_valid_moves", log->percent_valid_moves);
  assign_to_dict(dict, "opponent_final_hp", log->opponent_final_hp);

  // Added Squared metrics
  assign_to_dict(dict, "perf", log->perf);
  assign_to_dict(dict, "score", log->score);
  assign_to_dict(dict, "episode_return", log->episode_return);
  assign_to_dict(dict, "episode_length", log->episode_length);
  assign_to_dict(dict, "n", log->n);
  assign_to_dict(dict, "6v6_wr", log->six_wins);
  assign_to_dict(dict, "OU_wr", log->gen1_wins);
  
  return 0;
}
