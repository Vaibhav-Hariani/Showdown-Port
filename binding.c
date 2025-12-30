#include "sim.h"

#define Env Sim
#include "../env_binding.h"

static int my_init(Env* env, PyObject* args, PyObject* kwargs) {
  // Default to 1 agent (P1 controlled by policy, P2 is AI)
  env->num_agents = 1;
  env->num_agents = unpack(kwargs, "num_agents");
  init_sim(env);
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
