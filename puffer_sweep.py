from pufferlib import pufferl

from pufferlib.models import Default
import pufferlib

import torch
from torch import nn
import wandb

wandb.login()

if __name__ == "__main__":
    args = pufferl.load_config("default")
    args["wandb"] = True
    args["package"] = 'ocean'
    args["policy"]["hidden_size"] = 256
    args["policy"]["depth"] = 12
    args["vec"]["num_envs"] = 12
    args["env"] = {"num_envs": 1024}
    args["policy_name"] = "Showdown"
    pufferl.sweep(args, env_name="puffer_showdown")
