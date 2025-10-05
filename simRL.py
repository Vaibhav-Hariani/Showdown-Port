from pufferlib import pufferl
from pufferlib.ocean.showdown.showdown import Showdown
from pufferlib.ocean.squared.squared import Squared

import pufferlib
from pufferlib.ocean.torch import ShowdownLSTM
from pufferlib.ocean.torch import Showdown as ShowdownModel
import torch
from torch import nn
import wandb
import numpy as np

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
    # Initialize WandB first
    args = pufferl.load_config("showdown")
    args["wandb"] = True
    args["package"] = 'ocean'
    args["policy"] = "puffer"
    args["train"]["env"] = "showdown"
    
    with wandb.init(project="showdown", config=args["train"]) as run:
        # Create artifact for episode observations (now stores WandB tables)
        artifact = wandb.Artifact(
            name="showdown_episodes",
            type="episode_tables",
            description="Pokemon battle episode recordings as tables from training"
        )
        env_args = [1024, 0, 1, 10, artifact]

        env = pufferlib.vector.make(
            Showdown,
            num_envs=1,
            env_args=env_args,
            batch_size=1,
            backend=pufferlib.vector.Serial,
        )
        
        env.reset(seed=42)
        policy = ShowdownModel(env.driver_env).cuda()
        args["train"]["use_rnn"] = True
        policy = ShowdownLSTM(env.driver_env, policy).cuda()
        
        trainer = pufferl.PuffeRL(args["train"], env, policy=policy)
        
        for epoch in range(100):
            # Run evaluation and training
            trainer.evaluate()
            logs = trainer.train()
            if logs:
                wandb.log(logs)
        trainer.print_dashboard()
        trainer.close()
        run.log_artifact(artifact)
