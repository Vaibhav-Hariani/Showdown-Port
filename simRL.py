from pufferlib import pufferl
from pufferlib.ocean.Showdown.sim import Sim
from pufferlib.models import Default
import pufferlib
import torch


##An embedding table, concatenated into the input for a linear model 
class ShowdownModel(torch.nn.Module):
    ##Embedding a Pokemon, 2 types, (move_id) x 4: 7 inputs
    def __init__(self, env):
        super().__init__()
        self.env = env        
        ##Run through this only if the env has the terminal flag set
        ##Max embedding is 165, for maximum move value.
        ##Roughly a 4d dimensor should be sufficient.
        self.embed = torch.nn.Embedding(165, embedding_dim=4)
        self.flatten = torch.nn.Flatten()
        self.linear = torch.nn.Linear(4, 10)
        self.embedding_cache = None
    
    def __call__(self, x):
        # emb = self.embedding_cache
        # if self.env.terminal or emb is None:
        emb = self.embed(x)  # Shape: [batch_size, 5, 4]
        emb = torch.sum(emb, dim=1)  # Sum across the sequence dimension -> [batch_size, 4]
        self.embedding_cache = emb
        emb = torch.flatten()
        x = self.linear(emb)  # Final projection -> [batch_size, 10]
        return x

# from pufferlib.vector import autotune
if __name__ == "__main__":
    multiproc_vecenv = pufferlib.vector.make(Sim, num_envs=8, env_args=[1024], batch_size=4, backend=pufferlib.vector.Multiprocessing)
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