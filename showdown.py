import gymnasium
import numpy as np
from pufferlib.ocean.showdown import binding
from pufferlib.ocean.showdown.py_print import ShowdownParser
import pufferlib
import wandb

class Showdown(pufferlib.PufferEnv):
    def __init__(
        self, num_envs=1, render_mode=None, log_interval=10, artifact_interval=None,
        artifact_collection=None, buf=None, seed=0
    ):
        # Observation layout now begins with an action row:
        # [ P1_action_type, P1_action_value, P2_action_type, P2_action_value, + 5 pad] (9 ints)
        # Followed by per-Pokemon rows: id, move1..4, hp, status_flags, stat_mod1, stat_mod2 (9 ints each)
        # Total length = (1 + 2*6) * 9. With NUM_POKE=1 => 117
        self.single_observation_space = gymnasium.spaces.Box(
            low=-32768, high=32767, shape=(117,), dtype=np.int16
        )
        self.single_action_space = gymnasium.spaces.Discrete(10)
        self.render_mode = render_mode
        self.num_agents = num_envs
        self.log_interval = log_interval

        self.artifact_interval = artifact_interval
        # WandB Artifact (misnamed collection)
        self.artifact_collection = artifact_collection
        self.artifact_episode = False

        if artifact_interval and artifact_collection:
            self.episode_observations = []
            self.episode_actions = []
            self.episode_rewards = []
            # Capture the first episode by default so users see immediate data
            self.artifact_episode = True

        self.num_games = 0
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
            # Reset a specific environmentin step, on every thousan
            binding.env_reset(self.c_envs + options, seed)
        binding.vec_reset(self.c_envs, seed)
        self.tick = 0
        return self.observations, []

    def step(self, actions):
        self.tick += 1
        self.actions[:] = actions
        binding.vec_step(self.c_envs)
        info = []
        if self.artifact_episode:
            # Store observation and action for environment 0
            self.episode_observations.append(self.observations[0].copy())
            self.episode_actions.append(int(actions[0]))
            self.episode_rewards.append(float(self.rewards[0]))

        if self.terminals[0] == True:
            self.num_games += 1
            self.tick = 0
            # Standard logging at log_interval
            if self.num_games % self.log_interval == 0:
                log = binding.vec_log(self.c_envs)
                info.append(log)
            # End observation logging if active
            if self.artifact_episode:
                # Log final observation and reward for env 0
                self.episode_observations.append(self.observations[0].copy())
                self.episode_actions.append(-1)  # No action on terminal step
                self.episode_rewards.append(float(self.rewards[0]))
                # Save to artifact collection
                self._save_to_artifact_collection()
                self.artifact_episode = False

            # Schedule next artifact capture (independent of whether we just captured)
            if (self.artifact_interval and self.artifact_collection and
                self.num_games % self.artifact_interval == 0):
                # Mark next episode for artifact capture
                self.artifact_episode = True

        return (self.observations, self.rewards, self.terminals, self.truncations, info)

    def _save_to_artifact_collection(self):
        """Save episode data to WandB artifact collection as a table"""
        # Create WandB table with parsed observations
        table = wandb.Table(columns=[
            "step", "action", "reward",
            "p1_choice", "p1_value", "p2_choice", "p2_value",
            "p1_active_id", "p1_active_name", "p1_active_hp", "p1_active_status", "p1_moves", "p1_move_names",
            "p2_active_id", "p2_active_name", "p2_active_hp", "p2_active_status", "p2_moves", "p2_move_names"
        ])

        # Parse and add each step efficiently
        for step_idx in range(len(self.episode_observations)):
            obs = self.episode_observations[step_idx]
            action = self.episode_actions[step_idx]
            reward = self.episode_rewards[step_idx]

            # Fast observation parsing - returns formatted strings
            parsed = ShowdownParser.parse_observation_fast(obs)
            p1 = parsed['p1_active']
            p2 = parsed['p2_active']

            table.add_data(
                step_idx,
                action,
                reward,
                int(obs[0]),
                int(obs[1]),
                int(obs[2]),
                int(obs[3]),
                p1['id'],
                p1['name'],
                p1['hp'],
                p1['status'],
                p1['moves'],
                p1['move_names'],
                p2['id'],
                p2['name'],
                p2['hp'],
                p2['status'],
                p2['moves'],
                p2['move_names']
            )

        # Add table to artifact
        # Use .table.json extension so artifact browser infers media type
        if self.artifact_collection is not None:
            self.artifact_collection.add(table, f"episode_{self.num_games:04d}.table.json")

        # Clear episode data
        self.episode_observations = []
        self.episode_actions = []
        self.episode_rewards = []

    def render(self):
        binding.vec_render(self.c_envs, 0)
        # return ShowdownParser.pretty_print(self.observations)

    def close(self):
        binding.vec_close(self.c_envs)


if __name__ == "__main__":
    N = 24
    env = Showdown(num_envs=N)
    env.reset(seed=42)
    steps = 0
    CACHE = 1024 * N
    actions = np.random.randint(6, 9, (CACHE, N))
    i = 0
    import time

    print("Starting now")
    start = time.time()
    info = None
    while time.time() - start < 10:
        obs, rewards, terminals, trunc, info_tmp = env.step(actions[i % CACHE])
        # obs_dict = parse_observation(obs[0])
        # min_obs(obs_dict)
        steps += N
        i += 1
        if info_tmp:
            info = info_tmp
        # print(steps, 'steps in', time.time() - start, 'seconds', end='\r')
    print("\n Showdown SPS:", int(steps / (time.time() - start)))
