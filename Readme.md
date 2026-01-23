A port of the Pokemon Showdown Simulator to C, for the Poke Agent Competition. Performance focused for applications in reinforcement learning.
Built with a target of Gen1OU. 

Our thesis revolved around replacing an imitatation learning phase with significantly longer training runs, to explore emergent behavior that differs from Human play.
To accomplish this, Showdown was replaced with our simulator, enabling a nearly 2000x speedup. 

This repo is designed to be plug and play with pufferlib ocean, and will be archived as we eventually become a part of [Pufferlib](https://github.com/PufferAI/PufferLib). In this state, the simulator currently has significant deviations from showdown damage calculation, problems that we need to rectify. 
