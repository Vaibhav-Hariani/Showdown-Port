#include "sim.h"

#define Env Sim
#include "../env_binding.h"

static int my_init(Env* env, PyObject* args, PyObject* kwargs) {
  // env->size = unpack(kwargs, "size");
  // init(env);
  return 0;
}

static int my_log(PyObject* dict, Log* log) {
  assign_to_dict(dict, "num_games", log->num_games);
  assign_to_dict(dict, "num_moves", log->num_moves);
  assign_to_dict(dict, "num_won", log->num_won);
  assign_to_dict(dict, "num_lost", log->num_lost);
  assign_to_dict(dict, "valid_moves", log->valid_moves);
  assign_to_dict(dict, "invalid_moves", log->invalid_moves);
  assign_to_dict(dict, "win_rate", log->win_rate);
  assign_to_dict(dict, "avg_game_len", log->avg_game_len);
  assign_to_dict(dict, "avg_win_len", log->avg_win_len);
  assign_to_dict(dict, "invalid_moves_pct", log->invalid_moves_pct);
  assign_to_dict(dict, "avg_damage_pct", log->avg_damage_pct);
  return 0;
}
