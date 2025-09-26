# from pufferlib import pufferl
# from pufferlib.models import Default
# from pufferlib.ocean.Showdown.sim import Sim
# import pufferlib
import torch
from torch import nn

import pufferlib.pytorch


class ShowdownModel(nn.Module):
    # Embedding pokemon IDs and moves, sum per pokemon, combine with gamestate
    def __init__(self, env, hidden_size=128):
        super().__init__()
        self.env = env

        self.input_size = 108
        self.embed_size = 4

        # 4 embedding vectors, 9 regular vectors per pokemon * 12 = 13 * 12 = 156
        self.total_size = 156

        # Max embedding is 165, for maximum move value.
        # 4d embedding dimension should be sufficient.
        self.embed = nn.Embedding(165, embedding_dim=4)
        self.hidden_size = hidden_size
        self.encoder = nn.Sequential(
            pufferlib.pytorch.layer_init(nn.Linear(156, hidden_size)),
            nn.GELU(),
        )

        self.num_actions = 10
        self.decoder = pufferlib.pytorch.layer_init(
            nn.Linear(hidden_size, self.num_actions), std=0.01
        )
        self.value = pufferlib.pytorch.layer_init(nn.Linear(hidden_size, 1), std=1)

    def forward(self, observations, state=None):
        return self.forward_eval(observations, state)

    def forward_eval(self, observations, state=None):
        hidden = self.encode_observations(observations, state=state)
        logits, values = self.decode_actions(hidden)
        return logits, values

    def encode_observations(self, observations: torch.Tensor, state=None):
        mat = observations.view(-1, 12, 9)

        # All of this can can be cached during inference
        embed_in = mat[:, :, 0:5] & 0xFF
        embeddings = self.embed(embed_in).sum(dim=2)

        # Also encoding active pokemon here
        active_and_pps = mat[:, :, 0:5] & ~0xFF
        gamestate = mat[:, :, 5:]

        # Dimensions should be batch x (48 + 9 * 12) = 156
        combined_input = torch.cat(
            [embeddings, active_and_pps, gamestate], dim=2
        ).flatten(start_dim=1)

        return self.encoder.forward(combined_input)

    def decode_actions(self, hidden: torch.Tensor):
        logits = self.decoder.forward(hidden)
        values = self.value.forward(hidden)
        return logits, values


def test_model(n=12):
    """Build a synthetic batch of ``n`` identical Pokémon entries and run the model."""
    # Each Pokémon vector encodes the id, four moves, and four extra slots (e.g. PP/hp/status)
    test_vec = torch.tensor([155, 1, 2, 3, 4, 0, 0, 0, 0], dtype=torch.int)
    pokemon_matrix = test_vec.repeat(n, 1).flatten()
    test_vec[0] *= -1  # Mark active

    model = ShowdownModel(None)
    vec = model(pokemon_matrix.unsqueeze(0))
    print(vec)


# from pufferlib.vector import autotune
if __name__ == "__main__":
    test_model()

    # multiproc_vecenv = pufferlib.vector.make(
    #     Sim,
    #     num_envs=8,
    #     env_args=[1024],
    #     batch_size=4,
    #     backend=pufferlib.vector.Multiprocessing,
    # )
    # env = Sim(num_envs=8192)
    # env.reset(seed=42)
    # model = ShowdownModel(env)
    # test_vec = torch.tensor([[155, 1, 2, 3, 4]])
    # vec = model(test_vec)
    # print(vec)

    # ## Pokemon number, move1,move2, move3, move4 + PP for each, hp, status effects
    # num_rows = 16
    # policy = torch.nn.Embedding(num_rows, int(num_rows**0.25), device="cuda")

    # policy = Default(multiproc_vecenv.driver_env).cuda()
    # args = pufferl.load_config("default")
    # args["train"]["env"] = "Showdown"
    # # print(args)
    # # args['train']['compile'] = True
    # # args['train']['batch_size'] = 12288
    # args["train"]["minibatch_size"] = 32768
    # trainer = pufferl.PuffeRL(args["train"], multiproc_vecenv, policy=policy)
    # for epoch in range(10):
    #     trainer.evaluate()
    #     logs = trainer.train()
    # trainer.print_dashboard()
    # trainer.close()
