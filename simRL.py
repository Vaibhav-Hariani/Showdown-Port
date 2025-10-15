from pufferlib import pufferl
from pufferlib.ocean.showdown.showdown import Showdown, evaluate_model
from pufferlib.ocean.squared.squared import Squared

import pufferlib
from pufferlib.ocean.showdown_models import Showdown as ShowdownModel
from pufferlib.ocean.showdown_models import ShowdownLSTM
import torch
from torch import nn
import wandb
import numpy as np

# wandb.login()


def get_params(x):
    return [p for p in x.parameters()]


def test_model(n=12):
    """Build a synthetic batch of ``n`` identical Pokémon entries and run the model."""
    # Create proper 92-element observations (8 header + 12*7 pokemon data)
    # Header: [p1_choice, p1_val, p2_choice, p2_val, p1_statmods1, p1_statmods2, p2_statmods1, p2_statmods2]
    header = torch.zeros(8, dtype=torch.int16)
    # Pokemon rows: [species_id, move1, move2, move3, move4, hp_pack, status_bits] * 12
    pokemon_row = torch.tensor([155, 1, 2, 3, 4, 100, 0], dtype=torch.int16)  # Basic pokemon data
    pokemon_data = pokemon_row.repeat(12, 1)  # 12 pokemon
    
    # Create full observation
    observation = torch.cat([header, pokemon_data.flatten()])  # Shape: (92,)
    
    # Create batch of n identical observations
    batch_obs = observation.unsqueeze(0).repeat(n, 1)  # Shape: (n, 92)
    
    # Create model (without env for testing)
    model = ShowdownModel(None)
    logits, values = model(batch_obs.float())
    print(f"Input shape: {batch_obs.shape}")
    print(f"Logits shape: {logits.shape}, Values shape: {values.shape}")
    print(f"Sample logits: {logits[0]}")
    print(f"Sample value: {values[0]}")


# from pufferlib.vector import autotune
if __name__ == "__main__":
    # Initialize WandB first
    args = pufferl.load_config("showdown")
    args["wandb"] = True
    args["package"] = 'ocean'
    args["policy"] = "puffer"
    args["train"]["env"] = "showdown"
    args["train"]["minibatch_size"] = 64
    args["train"]["use_rnn"] = False
    # args["train"]["device"] = "cpu"
    # env_args = [1024]
    # env = pufferlib.vector.make(
    #     Showdown,
    #     num_envs=24,
    #     env_args=env_args,
    #     batch_size=24,  # Match num_envs to avoid batch synchronization issues
    #     backend=pufferlib.vector.Multiprocessing,
    # )
    # env.reset(seed=42)
    N = 1
    env = Showdown(num_envs=N)

    policy = ShowdownModel(env).cuda()
    # policy = ShowdownLSTM(env.driver_env, policy).cuda() 

    with wandb.init(project="showdown", config=args["train"]) as run:
        # args["train"]["use_rnn"] = True
        trainer = pufferl.PuffeRL(args["train"], env, policy=policy)
        
        for epoch in range(100):
            # Run evaluation and training
            trainer.evaluate()
            logs = trainer.train()
            if logs:
                wandb.log(logs)
        trainer.print_dashboard()
        trainer.close()        
        ##generate wandb artifacts to step through model
        evaluate_model(run, model=policy, config=args["train"], num_games=100)
