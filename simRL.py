from pufferlib import pufferl
from pufferlib.ocean.Showdown.sim import Sim
from pufferlib.models import Default

if __name__ == "__main__":
    N = 10
    env_name = 'Showdown'

    env = Sim(num_envs=N)
    env.reset(seed=42)

    policy = Default(env).cuda()
    args = pufferl.load_config('default')
    args['train']['env'] = env_name
    args['train']['minibatch_size'] = 64
    trainer = pufferl.PuffeRL(args['train'], env, policy=policy)
    for epoch in range(10):
        trainer.evaluate()
        logs = trainer.train()
    trainer.print_dashboard()
    trainer.close()