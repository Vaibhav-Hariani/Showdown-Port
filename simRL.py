from pufferlib import pufferl
from pufferlib.ocean.showdown.showdown import Showdown
from pufferlib.ocean.squared.squared import Squared

from pufferlib.models import Default
import pufferlib
from pufferlib.ocean.torch import Showdown as ShowdownModel
import torch
from torch import nn
import wandb

# wandb.login()


def get_params(x):
    return [p for p in x.parameters()]


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
    # env = pufferlib.vector.make(
    #     Showdown,
    #     num_envs=12,
    #     env_args=[1024,0,10], 
    #     batch_size=4,
    #     backend=pufferlib.vector.Multiprocessing,
    # )
    # env.reset(seed=42)
    env = Showdown(num_envs=1)
    args = pufferl.load_config("default")
    args["wandb"] = True
    args["package"] = 'ocean'
    args["policy"] = "puffer"
    # args["train"]["batch_size"] = 32768
    args["train"]["minibatch_size"] = 64 

    # args["train"]["optimizer"] = "adam"
    args["train"]["env"] = "showdown"
    args["train"]["amp"] = False

    policy = ShowdownModel(env.driver_env).cuda()
    trainer = pufferl.PuffeRL(args["train"], env, policy=policy)
    # with wandb.init(project="squared", config=args["train"],id="squared-logging") as run:
    for epoch in range(1000):
        trainer.evaluate()
        logs = trainer.train()
    trainer.print_dashboard()
    trainer.close()