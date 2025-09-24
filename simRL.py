from pufferlib import pufferl
from pufferlib.ocean.Showdown.sim import Sim
from pufferlib.models import Default

import torch

# from pufferlib.vector import autotune
if __name__ == "__main__":
    # autotune(Sim, batch_size=96, time_per_test=5)

    env = Sim(num_envs=8192)
    env.reset(seed=42)

    ## Pokemon number, type1, type2, move1,move2, move3, move4 + PP for each, hp, status effects
    num_rows = 16
    policy = torch.nn.Embedding(num_rows, int(num_rows ** 0.25),device='cuda')

    policy = Default(env).cuda()
    args = pufferl.load_config('default')
    args['train']['env'] = "Showdown"
    # args['train']['compile'] = True
    # args['train']['batch_size'] = 12288
    # args['train']['minibatch_size'] = 24
    trainer = pufferl.PuffeRL(args['train'], env, policy=policy)
    for epoch in range(10):
        trainer.evaluate()
        logs = trainer.train()
    trainer.print_dashboard()
    trainer.close()