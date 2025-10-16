"""
Competition script for running trained PufferLib Showdown models against opponents.
Adapted to use the new eval() pattern from showdown.py.
"""

import asyncio
import torch
from test_env import RLAgent, SmartAgent, RandomPlayer


async def run_competition(model_path: str, opponent_type='random', n_battles=10, device='cpu'):
    """
    Run a competition between the RL agent and an opponent.
    
    Args:
        model_path: Path to the trained model checkpoint (.pt file)
        opponent_type: Type of opponent ('random' or 'smart')
        n_battles: Number of battles to play
        device: Device to run inference on ('cpu' or 'cuda')
        
    Returns:
        dict with win statistics
    """
    print(f"Loading RL agent from {model_path}...")
    rl_agent = RLAgent(
        model_path=model_path,
        device=device,
        battle_format="gen1randombattle"
    )
    
    print(f"Creating {opponent_type} opponent...")
    if opponent_type == 'random':
        opponent = RandomPlayer(battle_format="gen1randombattle")
    elif opponent_type == 'smart':
        opponent = SmartAgent(battle_format="gen1randombattle")
    else:
        raise ValueError(f"Unknown opponent type: {opponent_type}")
    
    print(f"\nStarting {n_battles} battles...")
    
    # Track statistics
    wins = 0
    losses = 0
    ties = 0
    
    try:
        # Run battles
        await rl_agent.battle_against(opponent, n_battles=n_battles)
        
        # Analyze results
        for battle_tag, battle in rl_agent.battles.items():
            if battle.won:
                wins += 1
            elif battle.lost:
                losses += 1
            else:
                ties += 1
        
        total = wins + losses + ties
        win_rate = (wins / total * 100) if total > 0 else 0.0
        
        results = {
            'wins': wins,
            'losses': losses,
            'ties': ties,
            'total_games': total,
            'win_rate': win_rate,
        }
        
        # Print results
        print(f"\n=== Competition Results ===")
        print(f"Opponent: {opponent_type}")
        print(f"Total battles: {total}")
        print(f"Wins: {wins} ({win_rate:.1f}%)")
        print(f"Losses: {losses} ({losses/total*100 if total > 0 else 0:.1f}%)")
        print(f"Ties: {ties} ({ties/total*100 if total > 0 else 0:.1f}%)")
        
        return results
        
    except Exception as e:
        print(f"Error during competition: {e}")
        import traceback
        traceback.print_exc()
        return None


def main():
    """Main entry point for competition script."""
    import argparse
    
    parser = argparse.ArgumentParser(description="Run PufferLib Showdown model in competition")
    parser.add_argument('model_path', type=str, help='Path to trained model checkpoint')
    parser.add_argument('--opponent', type=str, default='random', 
                       choices=['random', 'smart'],
                       help='Type of opponent to battle against')
    parser.add_argument('--n-battles', type=int, default=10,
                       help='Number of battles to play')
    parser.add_argument('--device', type=str, default='cpu',
                       choices=['cpu', 'cuda'],
                       help='Device to run inference on')
    
    args = parser.parse_args()
    
    # Check if CUDA is available if requested
    if args.device == 'cuda' and not torch.cuda.is_available():
        print("Warning: CUDA requested but not available, falling back to CPU")
        args.device = 'cpu'
    
    # Run competition
    results = asyncio.run(run_competition(
        model_path=args.model_path,
        opponent_type=args.opponent,
        n_battles=args.n_battles,
        device=args.device
    ))
    
    if results:
        print(f"\nFinal win rate: {results['win_rate']:.1f}%")


if __name__ == '__main__':
    main()
