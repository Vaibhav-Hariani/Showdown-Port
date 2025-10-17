"""
Deployment script for poke-env agent configured for Gen1 OU laddering.
Mostly copied from test_env.py but sets username to PAC_PUFFER and queues for gen1OU.
Specify server URL via SHOWDOWN_SERVER_URL environment variable or pass as --server.
"""
import os
import asyncio
import time
import torch
import numpy as np

from poke_env.player import Player
from poke_env.teambuilder import Teambuilder

from move_labels import MOVE_LABELS
from pokedex_labels import POKEMON_NAMES
from ou_teams import OU_TEAMS, get_random_ou_team

from pufferlib.ocean.showdown_models import Showdown, ShowdownLSTM


class DeployTeambuilder(Teambuilder):
    def __init__(self):
        super().__init__()
        self.packed_teams = []
        for team_data in OU_TEAMS:
            showdown_team = self._convert_to_showdown_format(team_data)
            parsed = self.parse_showdown_team(showdown_team)
            self.packed_teams.append(self.join_team(parsed))

    def _convert_to_showdown_format(self, team_data):
        pokemon_strings = []
        for pokemon in team_data:
            species = pokemon['species']
            moves = pokemon['moves']
            s = f"{species}\n"
            for m in moves:
                s += f"- {m}\n"
            pokemon_strings.append(s)
        return "\n".join(pokemon_strings)

    def yield_team(self):
        import random
        return random.choice(self.packed_teams)


class DeployAgent(Player):
    def __init__(self, model_path: str, device='cuda', server_url=None, **kwargs):
        # Set username to PAC_PUFFER
        kwargs.setdefault('username', 'PAC_PUFFER')
        # Use DeployTeambuilder by default
        kwargs.setdefault('team', DeployTeambuilder())

        # Initialize model
        model = Showdown(None, hidden_size=512)
        self.model = ShowdownLSTM(None, policy=model, input_size=512, hidden_size=512)
        weights = torch.load(model_path, weights_only=False)
        self.model.load_state_dict(weights)
        self.model.to(device)
        self.model.eval()
        self.model.init_eval_state(device=device, batch_size=1)

        # Expose server URL if provided
        if server_url:
            kwargs['server_url'] = server_url

        super().__init__(**kwargs)

    # Minimal choose_move that mirrors RLAgent behaviour but simplified
    def choose_move(self, battle):
        # Return a random legal move or switch as fallback
        if battle.available_moves:
            return self.create_order(battle.available_moves[0])
        if battle.available_switches:
            return self.create_order(battle.available_switches[0])
        return self.choose_random_move(battle)


async def main(model_path: str, server_url: str | None = None, n_games: int = 10):
    # Build agent configured for ladder queue gen1OU at server_url
    agent = DeployAgent(model_path=model_path, device='cuda', server_url=server_url, battle_format='gen1ou')

    # Opponent: use random teams from OUTeambuilder to exercise generalization
    opponent = DeployAgent(model_path=model_path, device='cuda', team=get_random_ou_team(), battle_format='gen1ou')

    for i in range(n_games):
        await agent.battle_against(opponent, n_battles=1)


if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('--model', required=True, help='Path to model weights (.pt)')
    parser.add_argument('--server', default=os.environ.get('SHOWDOWN_SERVER_URL'), help='Showdown server URL')
    parser.add_argument('--games', type=int, default=10)
    args = parser.parse_args()

    # Run the deploy loop
    asyncio.run(main(model_path=args.model, server_url=args.server, n_games=args.games))
