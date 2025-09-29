import gymnasium
import numpy as np
from pufferlib.ocean.showdown import binding
import pufferlib
import torch

###############################
# Observation parsing helpers #
###############################

class Showdown(pufferlib.PufferEnv):
    def __init__(
        self, num_envs=1, render_mode=None, log_interval=128, buf=None, seed=0
    ):
        ##Dims for observations = 6 (Pokemon) * 2(Players) * 9 (Entries per pokemon) = 108
        # Each pokemon is represented by: [id, move1, move2, move3, move4, hp, status_flags, stat_mod1, stat_mod2]
        self.single_observation_space = gymnasium.spaces.Box(
            low=-32768, high=32767, shape=(108,), dtype=np.int16
        )
        self.single_action_space = gymnasium.spaces.Discrete(10)
        self.render_mode = render_mode
        self.num_agents = num_envs
        self.log_interval = log_interval

        super().__init__(buf)
        self.c_envs = binding.vec_init(
            self.observations,
            self.actions,
            self.rewards,
            self.terminals,
            self.truncations,
            num_envs,
            seed,
        )

    def reset(self, seed=0, options=None):
        if options:
            ## Reset a specific environment
            binding.env_reset(self.c_envs + options, seed)
        binding.vec_reset(self.c_envs, seed)
        self.tick = 0
        return self.observations, []

    def step(self, actions):
        self.tick += 1
        self.actions[:] = actions
        binding.vec_step(self.c_envs)
        info = []
        if self.tick % self.log_interval == 0:
            info.append(binding.vec_log(self.c_envs))
        if self.terminals[0] == True:
            # print("Episode finished after {} timesteps".format(self.tick))
            self.tick = 0
        ##Remove for actual work, might work for training the default env?
        # self.observations
        return (self.observations, self.rewards, self.terminals, self.truncations, info)

    def render(self):
        binding.vec_render(self.c_envs, 0)
        # return ShowdownParser.pretty_print(self.observations)

    def close(self):
        binding.vec_close(self.c_envs)


if __name__ == "__main__":
    N = 1
    env = Showdown(num_envs=N)
    env.reset(seed=42)
    steps = 0
    CACHE = 1024
    actions = np.random.randint(0, 10, (CACHE, N))
    i = 0
    import time

    print("Starting now")
    start = time.time()
    while time.time() - start < 10:
        obs, rewards, terminals, trunc, info = env.step(actions[i % CACHE])
        # obs_dict = parse_observation(obs[0])
        # min_obs(obs_dict)
        steps += N
        i += 1
        print(steps, 'steps in', time.time() - start, 'seconds', end='\r')
    print("\n Showdown SPS:", int(steps / (time.time() - start)))
