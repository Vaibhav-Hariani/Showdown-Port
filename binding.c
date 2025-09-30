#include "sim.h"

#define Env Sim
#include "../env_binding.h"

static int my_init(Env* env, PyObject* args, PyObject* kwargs) {
  // env->size = unpack(kwargs, "size");
  // init(env);
  return 0;
}

static int my_log(PyObject* dict, Log* log) {
  // Original Showdown metrics
  assign_to_dict(dict, "num_games", log->n);
  assign_to_dict(dict, "num_moves", log->num_moves);
  assign_to_dict(dict, "num_won", log->num_won);
  assign_to_dict(dict, "num_lost", log->num_lost);
  assign_to_dict(dict, "valid_moves", log->valid_moves);
  assign_to_dict(dict, "invalid_moves", log->invalid_moves);
  assign_to_dict(dict, "win_rate", log->win_rate);
  assign_to_dict(dict, "avg_damage_pct", log->avg_damage_pct);
  assign_to_dict(dict, "average_move_damage", log->team_strength);
  assign_to_dict(dict, "matchup_advantage", log->matchup_advantage);
  assign_to_dict(dict, "highest_damage_value", log->highest_damage_value);
  assign_to_dict(dict, "avg_opponent_poke_hp", log->opponent_avg_hp);

  // Added Squared metrics
  assign_to_dict(dict, "perf", log->perf);
  assign_to_dict(dict, "score", log->score);
  assign_to_dict(dict, "episode_return", log->episode_return);
  assign_to_dict(dict, "episode_length", log->episode_length);
  
  return 0;
}
