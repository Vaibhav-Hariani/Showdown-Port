from pufferlib import pufferl
import wandb

wandb.login()

if __name__ == "__main__":
    args = pufferl.load_config("showdown")
    args["wandb"] = True
    args["package"] = "ocean"

    args["tag"] = "showdown_sweep_bugfix"
    args["policy_name"] = "Showdown"
    args["rnn_name"] = "ShowdownLSTM"
    args["train"]["use_rnn"] = True
    pufferl.sweep(args, env_name="puffer_showdown")
