from pufferlib import pufferl
from pufferlib.ocean.showdown.showdown import Showdown
# from pufferlib.ocean.breakout.breakout import Breakout
# from pufferlib.ocean.squared.squared import Squared
from pufferlib.models import Default
import pufferlib
from pufferlib.ocean.torch import Showdown as ShowdownModel
import torch
from torch import nn
import wandb

wandb.login()


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
    env = pufferlib.vector.make(
        Showdown,
        num_envs=12,
        env_args=[1024],
        batch_size=4,
        backend=pufferlib.vector.Multiprocessing,
    )
    # env.reset(seed=42)
    args = pufferl.load_config("default")
    args["wandb"] = True
    args["package"] = 'ocean'
    args["policy"] = "puffer"
    # args["train"]["batch_size"] = 32768
    # args["train"]["optimizer"] = "adam"


    args["train"]["env"] = "Showdown"
    args["train"]["amp"] = False
    # args["train"]["learning_rate"] = 2.0e-4
    # pufferl.sweep(args, env_name="puffer_showdown")
    # policy = ShowdownModel(hidden_size=512, depth=12).cuda()

    with wandb.init(project="Showdown", config=args["train"]) as run:
        trainer = pufferl.PuffeRL(args["train"], env, policy=policy)
        for epoch in range(100):
            trainer.evaluate()
            logs = trainer.train()
            wandb.log(logs)
    trainer.print_dashboard()
    trainer.close()