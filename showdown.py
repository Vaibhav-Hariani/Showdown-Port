import gymnasium
import numpy as np
import os
import torch
from pufferlib.ocean.showdown import binding
from pufferlib.ocean.showdown.py_print import ShowdownParser
import pufferlib
import pufferlib.pytorch
from wandb import Table, Artifact


class Showdown(pufferlib.PufferEnv):
    def __init__(
            self, num_envs=1, render_mode=None, log_interval=1, buf=None, seed=0):
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


def log_episode(episode_obs, episode_actions, episode_rewards, game_count):
    """
    Create a wandb table for an episode.

    Args:
        episode_obs: List of observations for the episode
        episode_actions: List of actions taken
        episode_rewards: List of rewards received
        game_count: Game number for labeling

    Returns:
        tuple: (label, table) where label is the table name and table is the wandb Table object
    """
    final_reward = episode_rewards[-1]
    label = f"win #{game_count}" if final_reward > 0 else f"lose #{game_count}"

    # Create table (15 columns)
    table = Table(columns=[
        "step", "action", "reward",
        "p1_active_id", "p1_active_name", "p1_active_hp", "p1_active_status", "p1_moves", "p1_move_names",
        "p2_active_id", "p2_active_name", "p2_active_hp", "p2_active_status", "p2_moves", "p2_move_names"
    ])

    for step_idx in range(len(episode_obs)):
        obs_step = episode_obs[step_idx]
        action_step = episode_actions[step_idx]
        reward_step = episode_rewards[step_idx]

        parsed = ShowdownParser.parse_observation(obs_step, team_size=6)
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
            status_str = ','.join(
                [k for k, v in status.items() if v]) or 'healthy'
            moves_list = active.get('moves') or []
            moves_str = ','.join(
                [f"{m.get('id')}({m.get('pp')})" for m in moves_list])
            move_names = ','.join([m.get('name')
                                  for m in moves_list if m.get('name')])
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

    return label, table


def eval(policy, config, n_games=10):
    device = config['device'] 
    use_rnn = config['use_rnn']

    env = Showdown(num_envs=1, log_interval=n_games+1)
    ob, _ = env.reset()

    # Setup LSTM state if needed
    state = {}
    if use_rnn: 
        policy_hidden_size = policy.hidden_size
        state = dict(
            lstm_h=torch.zeros(1, policy_hidden_size, device=device),
            lstm_c=torch.zeros(1, policy_hidden_size, device=device),
        )

    # Statistics tracking
    games_completed = 0
    wins = 0
    losses = 0
    ties = 0
    game_len = 0
    game_rewards = []

    # Episode tracking for wandb tables
    episode_obs = []
    episode_actions = []
    episode_rewards = []
    tables_dict = {}
    
    policy.eval()
    # Run until we complete N games
    while games_completed < n_games:
        game_len += 1
        print(f"Evaluating game {games_completed}: On step {game_len}", end='\r')
        # Track observation
        episode_obs.append(ob[0].copy())

        with torch.no_grad():
            ob_tensor = torch.as_tensor(ob).to(device)
            logits, value = policy.forward_eval(ob_tensor, state)
            action, logprob, _ = pufferlib.pytorch.sample_logits(logits)

        # Track action
        action_cpu = int(action.cpu().numpy()[0])
        episode_actions.append(action_cpu)

        # Step environment
        ob, reward, done, truncated, info = env.step(action_cpu)

        # Track reward
        episode_rewards.append(float(reward[0]))

        # Check for game completion
        if done[0] or truncated[0]:
            # Extract final reward
            
            final_reward = float(reward[0])
            game_rewards.append(final_reward)

            # Count wins/losses/ties
            if final_reward > 0:
                wins += 1
            elif final_reward < 0:
                losses += 1
            else:
                ties += 1
            games_completed += 1
            # Create wandb table for this episode
            label, table = log_episode(
                episode_obs, episode_actions, episode_rewards, games_completed)
            tables_dict[label] = table

            # Reset episode tracking
            game_len = 0
            episode_obs = []
            episode_actions = []
            episode_rewards = []
            # Reset LSTM state if needed
            if use_rnn:
                state['lstm_h'].zero_()
                state['lstm_c'].zero_()

    # Cleanup
    env.close()

    # Compute statistics
    total_games = wins + losses + ties
    avg_reward = np.mean(game_rewards) if game_rewards else 0.0

    print(
        f"Of {total_games} games, {wins} wins, {losses} losses, {avg_reward} reward")
    return tables_dict


if __name__ == "__main__":
    N = 1
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
