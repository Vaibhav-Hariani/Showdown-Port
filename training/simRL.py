from pufferlib import pufferl
from pufferlib.ocean.showdown.showdown import Showdown
import pufferlib
from pufferlib.ocean.showdown_models import Showdown as ShowdownModel
from pufferlib.ocean.showdown_models import ShowdownLSTM
import wandb

wandb.login()


# from pufferlib.vector import autotune
if __name__ == "__main__":
    args = pufferl.load_config("showdown")
    args["wandb"] = True
    args["package"] = "ocean"
    args["policy"] = "puffer"
    args["train"]["env"] = "showdown"
    env_args = [1024, 2]
    env = pufferlib.vector.make(
        Showdown,
        num_envs=4,
        env_args=env_args,
        batch_size=4,
        backend=pufferlib.vector.Multiprocessing,
    )
    env.reset(seed=42)

    policy = ShowdownModel(env, hidden_size=512).cuda()
    args["train"]["use_rnn"] = True
    policy = ShowdownLSTM(
        env.driver_env, policy, input_size=512, hidden_size=512
    ).cuda()
    # base_path = f'/puffertank/Showdown/PufferLib/pufferlib/ocean/showdown/comp_env_bindings/'
    # model_name = "woven_field"
    # best_wr = 0

    # policy.load_state_dict(torch.load(f'{base_path}{model_name}.pt'))
    # with torch.no_grad():
    with wandb.init(project="showdown", config=args["train"]) as run:
        trainer = pufferl.PuffeRL(args["train"], env, policy=policy)
        for epoch in range(100):
            trainer.evaluate()
            logs = trainer.train()
            if logs:
                wandb.log(logs)
        trainer.print_dashboard()
        trainer.close()
        # generate wandb artifacts to step through model
        print("Evaluating trained model over 30 games...")
        tables = eval(policy, config=args["train"], n_games=30)
        wandb.log(tables)
