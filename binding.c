#include "sim.h"

#define OBS_SIZE     88
#define NUM_ATNS     1
#define ACT_SIZES    {10}
#define OBS_TENSOR_T IntTensor

#define Env Sim
#include "vecenv.h"

void my_init(Env* env, Dict* kwargs) {
  int num_agents = (int)dict_get(kwargs, "num_agents")->value;
  if (num_agents < 1 || num_agents > 2) num_agents = 1;
  env->num_agents = num_agents;

  int gametype_limit = (int)dict_get(kwargs, "gametype_limit")->value;
  if (gametype_limit < 1 || gametype_limit > TEAM_CONFIG_MAX)
    gametype_limit = TEAM_CONFIG_MAX;
  env->max_gametype = gametype_limit;

  int opp_type = (int)dict_get(kwargs, "opp_type")->value;
  if (opp_type < 1 || opp_type > 3) opp_type = 3;
  env->opp_type = opp_type;

  sim_init(env);
}

void my_log(Log* log, Dict* out) {
  dict_set(out, "num_moves",           log->num_moves);
  dict_set(out, "num_won",             log->num_won);
  dict_set(out, "num_lost",            log->num_lost);
  dict_set(out, "percent_valid_moves", log->percent_valid_moves);
  dict_set(out, "opponent_final_hp",   log->opponent_final_hp);
  dict_set(out, "perf",                log->perf);
  dict_set(out, "score",               log->score);
  dict_set(out, "episode_return",      log->episode_return);
  dict_set(out, "episode_length",      log->episode_length);
  dict_set(out, "n",                   log->n);
  dict_set(out, "6v6_wr",              log->six_wins);
  dict_set(out, "OU_wr",               log->gen1_wins);
}
