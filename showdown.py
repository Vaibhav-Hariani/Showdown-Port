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
        # Observation layout: 8 header ints + 12*7 = 84 ints (rows) = 92 total
        # Header: [p1_prev_choice, p1_prev_val, p2_prev_choice, p2_prev_val, p1_statmods1, p1_statmods2, p2_statmods1, p2_statmods2]
        # Each row: [species_id, move1, move2, move3, move4, hp_pack, status_bits]
        self.single_observation_space = gymnasium.spaces.Box(
            low=-32768, high=32767, shape=(92,), dtype=np.int16
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
            "p1_statmods", "p2_statmods",
            "p1_active_id", "p1_active_name", "p1_active_hp", "p1_active_status", "p1_moves", "p1_move_names",
            "p2_active_id", "p2_active_name", "p2_active_hp", "p2_active_status", "p2_moves", "p2_move_names"
        ])

        # Parse and add each step efficiently
        for step_idx in range(len(self.episode_observations)):
            obs = self.episode_observations[step_idx]
            action = self.episode_actions[step_idx]
            reward = self.episode_rewards[step_idx]

            # Use v2 parser for new packed obs
            parsed = ShowdownParser.parse_observation(obs)
            header = parsed['header']
            p1 = parsed['players'][0]['team'][parsed['players'][0]['active_index']] if parsed['players'][0]['active_index'] is not None else {}
            p2 = parsed['players'][1]['team'][parsed['players'][1]['active_index']] if parsed['players'][1]['active_index'] is not None else {}

            table.add_data(
                step_idx,
                action,
                reward,
                header['p1_prev_choice'],
                header['p1_prev_val'],
                header['p2_prev_choice'],
                header['p2_prev_val'],
                header['p1_statmods'],
                header['p2_statmods'],
                p1.get('id', None),
                p1.get('name', None),
                p1.get('hp_scaled', None),
                ShowdownParser.format_status_fast(p1.get('status', {}), ["paralyzed", "burn", "freeze", "poison", "sleep", "confused"]),
                ', '.join([f"{m['id']}({m['pp']})" for m in p1.get('moves', []) if m['id']]) if p1 else None,
                ', '.join([m['name'] for m in p1.get('moves', []) if m.get('name')]) if p1 else None,
                p2.get('id', None),
                p2.get('name', None),
                p2.get('hp_scaled', None),
                ShowdownParser.format_status_fast(p2.get('status', {}), ["paralyzed", "burn", "freeze", "poison", "sleep", "confused"]),
                ', '.join([f"{m['id']}({m['pp']})" for m in p2.get('moves', []) if m['id']]) if p2 else None,
                ', '.join([m['name'] for m in p2.get('moves', []) if m.get('name')]) if p2 else None,
            )
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
