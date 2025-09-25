from pufferlib import pufferl
from pufferlib.ocean.Showdown.sim import Sim
from pufferlib.models import Default
import pufferlib
import torch
from torch import nn

# An embedding table, concatenated into the input for a linear model


class ShowdownModel(torch.nn.Module):
    # Embedding pokemon IDs and moves, sum per pokemon, combine with gamestate
    def __init__(self, env):
        super().__init__()
        self.env = env
        # Max embedding is 165, for maximum move value.
        # 4d embedding dimension should be sufficient.
        self.embed = torch.nn.Embedding(165, embedding_dim=4)
        self.flatten = torch.nn.Flatten()

        # Total input: 12 pokemon * (4 embedding dims + 8 gamestate values) = 144 values
        self.linear = torch.nn.Linear(144, 10)
        # Figure we could use something like a cache to prevent the embedding
        # from being recomputed every time

    def __call__(self, x):
        # Reshape to [batch_size, 12 pokemon, 9 values per pokemon]
        mat = x.reshape(-1, 12, 9)

        # All of this can can be cached during inference
        embed_in = mat[:, :, 0:5] & 0xFF  # Mask to lowest byte
        # Todo: Positional encoding scheme
        # Should be of dimension [batch_size, 48 (4 embed per pokemon * 12 pokemon)]
        embeddings = self.embed(embed_in).sum(dim=2).flatten(start_dim=2)

        move_pps = mat[::, 1:5] & ~0xFF
        gamestate = mat[:, :, 5:].flatten(start_dim=2)

        # Dimensions should be batch x 48 + 8 * 12 = 144
        combined_input = torch.cat([embeddings, move_pps, gamestate], dim=1)

        # Pass through output model
        output = self.linear(combined_input)  # Shape: [batch_size, 10]
        return output


# from pufferlib.vector import autotune
if __name__ == "__main__":
    multiproc_vecenv = pufferlib.vector.make(Sim, num_envs=8, env_args=[
                                             1024], batch_size=4, backend=pufferlib.vector.Multiprocessing)
    # env = Sim(num_envs=8192)
    # env.reset(seed=42)
    # model = ShowdownModel(env)
    # test_vec = torch.tensor([[155,1,2,3,4]])
    # vec = model(test_vec)
    # print(vec)

    # ## Pokemon number, type1, type2, move1,move2, move3, move4 + PP for each, hp, status effects
    # num_rows = 16
    # policy = torch.nn.Embedding(num_rows, int(num_rows ** 0.25),device='cuda')

    policy = Default(multiproc_vecenv.driver_env).cuda()
    args = pufferl.load_config('default')
    args['train']['env'] = "Showdown"
    # print(args)
    # args['train']['compile'] = True
    # args['train']['batch_size'] = 12288
    args['train']['minibatch_size'] = 32768
    trainer = pufferl.PuffeRL(args['train'], multiproc_vecenv, policy=policy)
    for epoch in range(10):
        trainer.evaluate()
        logs = trainer.train()
    trainer.print_dashboard()
    trainer.close()
