import gymnasium
import numpy as np
import os
import torch
import pufferlib
from pufferlib.ocean.showdown import binding


class Showdown(pufferlib.PufferEnv):
    def __init__(
            self, num_envs=1, num_agents=1, render_mode=None, log_interval=1, buf=None, seed=0):
        # Validate num_agents
        if num_agents != 1 and num_agents != 2:
            raise pufferlib.APIUsageError('Showdown supports at most 2 agents (player 1 and player 2)')
                
        # Observation layout v2: header (4 ints: p1_statmods_word1, p1_statmods_word2, p2_statmods_word1, p2_statmods_word2)
        # Followed by interleaved team data: 2 players * 6 pokemon * 7 ints each (id, move1, move2, move3, move4, hp_scaled, status_flags)
        # Total length = 4 + 84 = 88
        self.single_observation_space = gymnasium.spaces.Box(
            low=-32768, high=32767, shape=(88,), dtype=np.int16
        )
        self.single_action_space = gymnasium.spaces.Discrete(10)
        self.render_mode = render_mode
        self.num_agents = num_envs * num_agents  # Total agents across all envs
        # self.agents_per_env = num_agents
        self.log_interval = log_interval
        self.num_games = 0
        super().__init__(buf)
        
        # Create C env instances following the Convert pattern
        c_envs = []
        for i in range(num_envs):
            c_env = binding.env_init(
                self.observations[i*num_agents:(i+1)*num_agents],
                self.actions[i*num_agents:(i+1)*num_agents],
                self.rewards[i*num_agents:(i+1)*num_agents],
                self.terminals[i*num_agents:(i+1)*num_agents],
                self.truncations[i*num_agents:(i+1)*num_agents],
                seed + i,
                num_agents=num_agents  # Pass num_agents to C binding
            )
            c_envs.append(c_env)
        
        self.c_envs = binding.vectorize(*c_envs)

    def reset(self, seed=0, options=None):
        binding.vec_reset(self.c_envs, seed)
        self.tick = 0
        return self.observations, []

    def step(self, actions):
        self.tick += 1
        self.actions[:] = actions
        binding.vec_step(self.c_envs)
        info = []
        if self.terminals[0] == True:
            self.num_games += 1
            self.tick = 0
            # Standard logging at log_interval
            if self.num_games % self.log_interval == 0:
                log = binding.vec_log(self.c_envs)
                info.append(log)

        return (self.observations, self.rewards, self.terminals, self.truncations, info)

    def render(self):
        binding.vec_render(self.c_envs, 0)
        # return ShowdownParser.pretty_print(self.observations)

    def close(self):
        binding.vec_close(self.c_envs)


if __name__ == "__main__":
    N = 1
    num_agents = 2  # Test with 2 agents to see P2 moves
    env = Showdown(num_envs=N,num_agents=num_agents)

    env.reset(seed=42)
    steps = 0
    CACHE = 1024
    actions = np.random.randint(0, 9, (CACHE, env.num_agents))
    i = 0
    import time

    print("Starting now")
    start = time.time()
    info = None
    while time.time() - start < 10:
        obs, rewards, terminals, trunc, info_tmp = env.step(actions[i % CACHE])
        steps += env.num_agents
        i += 1
        if info_tmp:
            info = info_tmp
        # print('%s steps in %s seconds' % (steps, time.time() - start), end='\r')
    duration = time.time() - start
    sps = steps / duration
    ms_per_move = (1000.0 / sps)
    sps_str = f"{sps:,.0f}"
    ms_str = f"{ms_per_move:,.3f}"
    print(f"\n Showdown SPS: {sps_str}  |  ms/move: {ms_str}")

    # If you want to run evaluation here, add a call to evaluate_model and print the result.
    # Example (requires a model, run, config):
    # wins, losses = evaluate_model(run, model, config)
    # print(f"Wins: {wins}, Losses: {losses}")
