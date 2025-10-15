import gymnasium
import numpy as np
import os
from pufferlib.ocean.showdown import binding
from pufferlib.ocean.showdown.py_print import ShowdownParser
import pufferlib
from wandb import Table, Artifact


class Showdown(pufferlib.PufferEnv):
    def __init__(
            self, num_envs=1, render_mode=None, log_interval=10, buf=None, seed=0):
        # Observation layout v2: header (4 ints: p1_statmods_word1, p1_statmods_word2, p2_statmods_word1, p2_statmods_word2)
        # Followed by interleaved team data: 2 players * 6 pokemon * 7 ints each (id, move1, move2, move3, move4, hp_scaled, status_flags)
        # Total length = 4 + 84 = 88
        self.single_observation_space = gymnasium.spaces.Box(
            low=-32768, high=32767, shape=(88,), dtype=np.int16
        )
        self.single_action_space = gymnasium.spaces.Discrete(10)
        self.render_mode = render_mode
        self.num_agents = num_envs
        self.log_interval = log_interval
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


def log_episode_to_wandb(run, episode_obs, episode_actions, episode_rewards, game_count):
    """Log an episode to wandb as an artifact with win/lose label."""
    final_reward = episode_rewards[-1]
    label = f"win #{game_count}" if final_reward > 0 else f"lose #{game_count}"
    
    # Create table (19 columns expected by the logging consumer)
    table = Table(columns=[
        "step", "action", "reward",
        "p1_active_id", "p1_active_name", "p1_active_hp", "p1_active_status", "p1_moves", "p1_move_names",
        "p2_active_id", "p2_active_name", "p2_active_hp", "p2_active_status", "p2_moves", "p2_move_names"
    ])

    for step_idx in range(len(episode_obs)):
        obs_step = episode_obs[step_idx]
        action_step = episode_actions[step_idx]
        reward_step = episode_rewards[step_idx]

        parsed = ShowdownParser.parse_observation(obs_step, team_size=6)  # Updated to handle new observation space
        p1_dict = {
            'id': None,
            'name': None,
            'hp': 0,
            'status': 'fainted',
            'moves': '',
            'move_names': ''
        }

        p2_dict = p1_dict.copy()

        # Parser now returns players[].active_pokemon (or None) and header statmods
        p1_info = parsed.get('players', [None, None])[0]
        p2_info = parsed.get('players', [None, None])[1]

        p1_active = None
        p2_active = None
        if p1_info:
            p1_active = p1_info.get('active_pokemon')
        if p2_info:
            p2_active = p2_info.get('active_pokemon')

        def _format_active(active):
            if not active:
                return dict(id=None, name=None, hp=0, status='fainted', moves='', move_names='')
            pid = active.get('id')
            name = active.get('name')
            hp = active.get('hp_scaled')
            status = active.get('status') or {}
            status_str = ','.join([k for k, v in status.items() if v]) or 'healthy'
            moves_list = active.get('moves') or []
            moves_str = ','.join([f"{m.get('id')}({m.get('pp')})" for m in moves_list])
            move_names = ','.join([m.get('name') for m in moves_list if m.get('name')])
            return dict(id=pid, name=name, hp=hp, status=status_str, moves=moves_str, move_names=move_names)

        p1_dict = _format_active(p1_active)
        p2_dict = _format_active(p2_active)

        table.add_data(
            step_idx,
            action_step,
            reward_step,
            p1_dict['id'],
            p1_dict['name'],
            p1_dict['hp'],
            p1_dict['status'],
            p1_dict['moves'],
            p1_dict['move_names'],
            p2_dict['id'],
            p2_dict['name'],
            p2_dict['hp'],
            p2_dict['status'],
            p2_dict['moves'],
            p2_dict['move_names']
        )
    # Log the completed table once per episode
    run.log({label: table})

    # # Create artifact and add table
    # artifact = Artifact(f"game_{game_count}_{label}", type="episode_data")
    # artifact.add(table, f"game_{game_count}_table")


def evaluate_model(run, model, config, num_games=1000):
    """Evaluate a model (either from wandb artifact path or policy object) by running it in serial for num_games and logging win/lose stats. Returns (num_wins, num_losses)."""
    import torch
    device = config['device']
    model.eval()
    env_args = [1024]
    env = pufferlib.vector.make(
        Showdown,
        num_envs=1,
        env_args=env_args,
        batch_size=1,
        backend=pufferlib.vector.Serial,
    )
    obs, _ = env.reset()
    game_count = 0
    num_wins = 0
    num_losses = 0
    while game_count < num_games:
        if config['use_rnn']:
            state = dict(
                lstm_h=torch.zeros(
                    obs.shape[0], model.hidden_size, device=device),
                lstm_c=torch.zeros(
                    obs.shape[0], model.hidden_size, device=device),
            )
        else:
            state = {}
        episode_obs = []
        episode_actions = []
        episode_rewards = []
        done = False
        while not done:
            obs_tensor = torch.as_tensor(obs).to(device)
            with torch.no_grad():
                logits, value = model.forward_eval(obs_tensor, state)
                action, logprob, _ = pufferlib.pytorch.sample_logits(logits)
            action = action.cpu().numpy().reshape(env.action_space.shape)
            obs, reward, terminal, truncation, info = env.step(action)
            episode_obs.append(obs[0].copy())
            episode_actions.append(int(action[0]))
            episode_rewards.append(float(reward[0]))
            if terminal[0]:
                done = True
                game_count += 1
                # Only count win/loss, do not log episode artifacts
                win = int(episode_rewards[-1] > 0)
                loss = int(episode_rewards[-1] <= 0)
                num_wins += win
                num_losses += loss
                # Log episode_obs to wandb
                log_episode_to_wandb(run, episode_obs, episode_actions, episode_rewards, game_count)
                obs, _ = env.reset()
    env.close()
    print(f"Evaluation complete: {num_wins} wins, {num_losses} losses out of {num_games} games.")
    # Save the model as a wandb artifact
    # Write into the comp_env folder so exported artifacts live with the compatibility
    # environment (required when loading in an independent env).
    model_path = os.path.abspath(os.path.join(os.getcwd(), "comp_env", "final_model.pt"))
    os.makedirs(os.path.dirname(model_path), exist_ok=True)
    torch.save(model.state_dict(), model_path)
    artifact = Artifact("final_model", type="model")
    artifact.add_file(model_path)
    run.log_artifact(artifact)
    return num_wins, num_losses


if __name__ == "__main__":
    N = 24
    env = Showdown(num_envs=N)
    env.reset(seed=42)
    steps = 0
    CACHE = int(1e6 * N)
    actions = np.random.randint(6, 9, (CACHE, N))
    i = 0
    import time

    print("Starting now")
    start = time.time()
    info = None
    while time.time() - start < 10:
        obs, rewards, terminals, trunc, info_tmp = env.step(actions[i % CACHE])
        steps += N
        i += 1
        if info_tmp:
            info = info_tmp
        # print('%s steps in %s seconds' % (steps, time.time() - start), end='\r')
    duration = time.time() - start
    sps = steps / duration if duration > 0 else 0
    ms_per_move = (1000.0 / sps) if sps > 0 else 0.0
    sps_str = f"{sps:,.0f}"
    ms_str = f"{ms_per_move:,.3f}"
    print(f"\n Showdown SPS: {sps_str}  |  ms/move: {ms_str}")
    # If you want to run evaluation here, add a call to evaluate_model and print the result.
    # Example (requires a model, run, config):
    # wins, losses = evaluate_model(run, model, config)
    # print(f"Wins: {wins}, Losses: {losses}")
