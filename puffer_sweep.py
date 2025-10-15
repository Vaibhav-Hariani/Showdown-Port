from pufferlib import pufferl
from pufferlib.ocean.showdown.showdown import Showdown
from pufferlib.ocean.torch import ShowdownLSTM as ShowdownModel


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
    args["vec"]["num_envs"] = 24
    args["env"] = {"num_envs": 1024}
    args["tag"] = "showdown_6v6_teamgen"
    args["policy_name"] = "Showdown"
    args["rnn_name"] = "ShowdownLSTM"
    args["train"]["use_rnn"] = True
    pufferl.sweep(args, env_name="puffer_showdown")
