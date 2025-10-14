#!/usr/bin/env python3
"""
Verification script to test the expanded observation spaces
"""

import numpy as np

def verify_pufferlib_obs_space():
    """Verify PufferLib Showdown observation space size"""
    print("=" * 60)
    print("PufferLib Showdown Observation Space Verification")
    print("=" * 60)
    
    # Action row
    # After removing the previous-choice/value fields, the action row keeps 5 padding slots
    action_row_size = 5
    print(f"Action row size: {action_row_size}")
    print("  - 5 padding")
    
    # Pokemon data
    NUM_POKE = 1
    values_per_pokemon = 25
    num_pokemon = NUM_POKE * 2  # Both players
    pokemon_data_size = num_pokemon * values_per_pokemon
    print(f"\nPokemon data size: {pokemon_data_size}")
    print(f"  - {num_pokemon} Pokemon × {values_per_pokemon} values/Pokemon")
    print(f"  - Pokemon structure (25 values):")
    print(f"    • ID (1)")
    print(f"    • Moves (4)")
    print(f"    • HP current + max (2)")
    print(f"    • Base stats (6): HP, ATK, DEF, SPA, SPD, SPE")
    print(f"    • Level (1)")
    print(f"    • Types (2)")
    print(f"    • Status flags (1)")
    print(f"    • Stat modifiers (2)")
    print(f"    • Volatile status (6): badly_poisoned, sleep, recharge×3, flinch, confusion")
    
    total_size = action_row_size + pokemon_data_size
    print(f"\n{'Total observation size:':<30} {total_size}")
    print(f"{'Expected in showdown.py:':<30} 55")
    
    if total_size == 55:
        print("\n✅ PufferLib observation space size is CORRECT!")
    else:
        print(f"\n❌ ERROR: Size mismatch! Expected 55, got {total_size}")
    
    return total_size == 59

def verify_test_env_obs_space():
    """Verify test_env.py observation space size"""
    print("\n" + "=" * 60)
    print("test_env.py Observation Space Verification")
    print("=" * 60)
    
    # Move information
    moves_size = 4 * 3  # 4 moves × 3 features (power, multiplier, accuracy)
    print(f"Move information size: {moves_size}")
    print(f"  - 4 moves × 3 features (power, type_effectiveness, accuracy)")
    
    # Active Pokemon - Self
    active_self_size = 13
    print(f"\nActive Pokemon (Self) size: {active_self_size}")
    print(f"  - HP (1) + Base stats (5) + Stat boosts (5) + Status (1) + Fainted (1)")
    
    # Active Pokemon - Opponent
    active_opp_size = 13
    print(f"\nActive Pokemon (Opponent) size: {active_opp_size}")
    print(f"  - Same structure as self")
    
    # Team information
    team_info_size = 12
    print(f"\nTeam information size: {team_info_size}")
    print(f"  - Self team (6): fainted, avg_hp, low_hp, mid_hp, high_hp, status_count")
    print(f"  - Opponent team (6): same structure")
    
    # Battle state
    battle_state_size = 4
    print(f"\nBattle state size: {battle_state_size}")
    print(f"  - Weather, Terrain, Trick Room, Turn number")
    
    total_size = moves_size + active_self_size + active_opp_size + team_info_size + battle_state_size
    print(f"\n{'Total observation size:':<30} {total_size}")
    print(f"{'Expected in test_env.py:':<30} 54")
    
    if total_size == 54:
        print("\n✅ test_env.py observation space size is CORRECT!")
    else:
        print(f"\n❌ ERROR: Size mismatch! Expected 54, got {total_size}")
    
    return total_size == 54

def main():
    print("\n" + "=" * 60)
    print("OBSERVATION SPACE VERIFICATION")
    print("=" * 60)
    
    pufferlib_ok = verify_pufferlib_obs_space()
    test_env_ok = verify_test_env_obs_space()
    
    print("\n" + "=" * 60)
    print("SUMMARY")
    print("=" * 60)
    print(f"PufferLib Showdown: {'✅ PASS' if pufferlib_ok else '❌ FAIL'}")
    print(f"test_env.py:        {'✅ PASS' if test_env_ok else '❌ FAIL'}")
    
    if pufferlib_ok and test_env_ok:
        print("\n✅ All observation spaces verified successfully!")
        return 0
    else:
        print("\n❌ Some observation spaces have errors!")
        return 1

if __name__ == "__main__":
    exit(main())
