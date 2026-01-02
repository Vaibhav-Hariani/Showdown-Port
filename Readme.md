A port of the Pokemon Showdown Simulator to C, for the Poke Agent Competition. Performance focused for applications in reinforcement learning.
Built with a target of Gen1OU. 

Our thesis revolved around replacing an imitatation learning phase with significantly longer training runs, to explore emergent behavior that differs from Human play.
To accomplish this, Showdown was replaced with our simulator, enabling a nearly 2000x speedup. 

Our architecture is currently an LSTM, though future research would involve exploring alternative models and techniques with our simulator.
