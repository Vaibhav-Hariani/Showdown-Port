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
    args = pufferl.load_config("showdown")
    args["wandb"] = True
    args["package"] = 'ocean'

    args["tag"] = "showdown_final_sweep"
    args["policy_name"] = "Showdown"
    args["rnn_name"] = "ShowdownLSTM"
    args["train"]["use_rnn"] = True
    pufferl.sweep(args, env_name="puffer_showdown")
