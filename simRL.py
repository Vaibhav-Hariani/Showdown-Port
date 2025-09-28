from pufferlib import pufferl
from pufferlib.ocean.showdown.showdown import Showdown
# from pufferlib.ocean.breakout.breakout import Breakout
# from pufferlib.ocean.squared.squared import Squared
from pufferlib.models import Default
import pufferlib

import torch
from torch import nn
import wandb

wandb.login()
def get_params(x):
    return [p for p in x.parameters()]


class ShowdownModel(nn.Module):
    # Embedding pokemon IDs and moves, sum per pokemon, combine with gamestate
    def __init__(self, hidden_size=128, depth=1):
        super().__init__()

        self.input_size = 108
        self.embed_size = 5

        # 4 embedding vectors, 9 regular vectors per pokemon * 12 = 13 * 12 = 156
        self.total_size = (self.embed_size + 9) * 12
        # Max embedding is 165, for maximum move value.
        # 4d embedding dimension should be sufficient.
        # self.embed = nn.Embedding(165, embedding_dim=4)
        self.embed = nn.Identity(5)
        self.hidden_size = hidden_size
        self.num_actions = 10

        encoder_layers = [nn.Linear(self.total_size, hidden_size), nn.GELU()]
        for i in range(depth):
            encoder_layers.append(nn.Linear(hidden_size, hidden_size))
            encoder_layers.append(nn.GELU())

        self.encoder = nn.Sequential(*encoder_layers)
        self.decoder = pufferlib.pytorch.layer_init(
            nn.Linear(hidden_size, self.num_actions), std=0.01
        )
        self.value = pufferlib.pytorch.layer_init(nn.Linear(hidden_size, 1), std=1)

    def forward(self, observations, state=None):
        ##print for the very first batch 
        # ShowdownParser.pretty_print(observations[0].cpu().numpy())
        return self.forward_eval(observations, state)


    def forward_eval(self, observations, state=None):
        hidden = self.encode_observations(observations, state=state)
        logits, values = self.decode_actions(hidden)
        return logits, values

    def encode_observations(self, observations: torch.Tensor, state=None):
        mat = observations.view(-1, 12, 9)

        # All of this can can be cached during inference
        # embed_in = torch.zeros_like(mat[:, :, 0:5],dtype=float)
        embed_in = (torch.abs(mat[:, :, 0:5]) & 0xFF).float()
        # embed_in[:,:,0] = torch.abs(embed_in[:,:,0]) / 155
        # embed_in[:,:,1:5] = embed_in[:,:,1:5] / 165

        embed_in = embed_in / 255

        embeddings = self.embed(embed_in[0:])
        # embeddings = embeddings.sum(dim=2)

        # Also encoding active pokemon here
        active_and_pps = ((mat[:, :, 0:5] & ~0xFF) >> 8) / 255
        gamestate = (mat[:, :, 5:]) / 800
        # Dimensions should be batch x (48 + 9 * 12) = 156
        combined_input = torch.cat(
            [embeddings, active_and_pps, gamestate], dim=2
        ).flatten(start_dim=1)
        if(combined_input.max() > 1 or combined_input.min() < -1):
            print("Warning: combined input out of range", combined_input.max(), combined_input.min())
        out =  self.encoder.forward(combined_input)
        if(out.isnan().any()):
            pass
        return out

    def decode_actions(self, hidden: torch.Tensor):
        logits = self.decoder.forward(hidden)
        values = self.value.forward(hidden)
        if(logits.isnan().any() or values.isnan().any()):
            pass
        return logits, values


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
    # args["train"]["batch_size"] = 32768
    args["train"]["optimizer"] = "adam"
    args["train"]["env"] = "Showdown"
    # pufferl.sweep(args, env_name="puffer_showdown")
    policy = ShowdownModel(hidden_size=512, depth=12).cuda()

    with wandb.init(project="Showdown", config=args["train"],id="Showdown-adam-hidden-512") as run:
        trainer = pufferl.PuffeRL(args["train"], env, policy=policy)
        for epoch in range(100):
            trainer.evaluate()
            logs = trainer.train()
            wandb.log(logs)
    trainer.print_dashboard()
    trainer.close()