from pufferlib import pufferl
from pufferlib.models import Default
from pufferlib.ocean.Showdown.sim import Sim
import pufferlib
import torch
from torch import nn


class ShowdownModel(nn.Module):
    # Embedding pokemon IDs and moves, sum per pokemon, combine with gamestate
    def __init__(self, env):
        super().__init__()
        self.env = env
        # Max embedding is 165, for maximum move value.
        # 4d embedding dimension should be sufficient.
        self.embed = nn.Embedding(165, embedding_dim=4)
        # Total input: 12 pokemon * (4 embedding dims + 8 gamestate values) = 144 values
        self.linear = nn.Linear(156, 10)

    def __call__(self, x: torch.Tensor, num_poke=12):
        mat = x.reshape(-1, num_poke, 9)

        # All of this can can be cached during inference
        embed_in = mat[:, :, 0:5] & 0xFF
        # Apply embeddings
        embeddings = self.embed(embed_in).sum(dim=2)
        ##Also encoding active pokemon here
        active_and_pps = mat[:, :, 0:5] & ~0xFF
        gamestate = mat[:, :, 5:]

        # Dimensions should be batch x (48 + 9 * 12) = 156
        combined_input = torch.cat(
            [embeddings, active_and_pps, gamestate], dim=2
        ).flatten(start_dim=1)

        # Pass through output model
        output = self.linear(combined_input)  # Shape: [batch_size, 10]
        return output


def test_model(n: int = 12):
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

    multiproc_vecenv = pufferlib.vector.make(
        Sim,
        num_envs=8,
        env_args=[1024],
        batch_size=4,
        backend=pufferlib.vector.Multiprocessing,
    )
    env = Sim(num_envs=8192)
    env.reset(seed=42)
    model = ShowdownModel(env)
    test_vec = torch.tensor([[155, 1, 2, 3, 4]])
    vec = model(test_vec)
    print(vec)

    ## Pokemon number, move1,move2, move3, move4 + PP for each, hp, status effects
    num_rows = 16
    policy = torch.nn.Embedding(num_rows, int(num_rows**0.25), device="cuda")

    policy = Default(multiproc_vecenv.driver_env).cuda()
    args = pufferl.load_config("default")
    args["train"]["env"] = "Showdown"
    # print(args)
    # args['train']['compile'] = True
    # args['train']['batch_size'] = 12288
    args["train"]["minibatch_size"] = 32768
    trainer = pufferl.PuffeRL(args["train"], multiproc_vecenv, policy=policy)
    for epoch in range(10):
        trainer.evaluate()
        logs = trainer.train()
    trainer.print_dashboard()
    trainer.close()
