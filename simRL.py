from pufferlib import pufferl
from pufferlib.ocean.Showdown.sim import Sim
from pufferlib.models import Default

import tomllib
if __name__ == "__main__":
    N = 10
    env_name = 'Showdown'

    env = Sim(num_envs=N)
    env.reset(seed=42)

    policy = Default(env)
    args = pufferl.load_config('default')
    args['train']['env'] = env_name
    print(args)
    model = pufferl.PuffeRL(args['train'], env, policy=policy)
    model.train()
