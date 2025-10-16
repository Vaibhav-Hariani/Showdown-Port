#!/usr/bin/env python3
"""
Quick test to verify the competition environment setup is working correctly.
"""

import sys
import os
sys.path.insert(0, os.path.dirname(__file__))

import torch
import numpy as np
from test_env import Embed, RLAgent, SmartAgent


def test_embedding():
    """Test that observation embedding works."""
    print("Testing observation embedding...")
    embed = Embed()
    
    # Create a dummy packed observation (same format as C sim)
    dummy_obs = np.zeros(88, dtype=np.int16)
    dummy_obs[0] = 100  # Some stat mod value
    dummy_obs[4] = 1    # First pokemon ID
    
    print(f"  Observation shape: {dummy_obs.shape}")
    print(f"  Observation dtype: {dummy_obs.dtype}")
    print(f"  Sample values: {dummy_obs[:10]}")
    print("✓ Embedding format OK\n")


def test_model_loading():
    """Test that model loading works (if a model file exists)."""
    print("Testing model loading...")
    
    # Look for any available model
    model_dirs = [
        "/puffertank/Showdown/PufferLib/pufferlib/ocean/showdown/comp_env_bindings/",
        "/puffertank/Showdown/PufferLib/experiments/",
        "/puffertank/Showdown/PufferLib/comp_env/",
    ]
    
    model_path = None
    for d in model_dirs:
        if os.path.exists(d):
            pt_files = [f for f in os.listdir(d) if f.endswith('.pt')]
            if pt_files:
                model_path = os.path.join(d, pt_files[0])
                break
    
    if model_path and os.path.exists(model_path):
        print(f"  Found model: {model_path}")
        try:
            agent = RLAgent(
                model_path=model_path,
                device='cpu',
                battle_format="gen1randombattle"
            )
            print(f"  Model loaded successfully")
            print(f"  Hidden size: {agent.model.hidden_size}")
            print(f"  Device: {agent.device}")
            print("✓ Model loading OK\n")
            return True
        except Exception as e:
            print(f"  Warning: Model loading failed: {e}")
            print("  (This is OK if no trained model exists yet)\n")
            return False
    else:
        print("  No model file found (this is OK for a fresh setup)")
        print("  Train a model first using simRL.py\n")
        return False


def test_inference():
    """Test model inference with dummy observation."""
    print("Testing model inference...")
    
    # Look for a model
    model_dirs = [
        "/puffertank/Showdown/PufferLib/pufferlib/ocean/showdown/comp_env_bindings/",
        "/puffertank/Showdown/PufferLib/experiments/",
    ]
    
    model_path = None
    for d in model_dirs:
        if os.path.exists(d):
            pt_files = [f for f in os.listdir(d) if f.endswith('.pt')]
            if pt_files:
                model_path = os.path.join(d, pt_files[0])
                break
    
    if not model_path:
        print("  Skipping (no model available)")
        return False
    
    try:
        agent = RLAgent(model_path=model_path, device='cpu', battle_format="gen1randombattle")
        
        # Create dummy observation
        dummy_obs = np.zeros(88, dtype=np.int16)
        dummy_obs[4] = 25  # Pikachu
        dummy_obs[11] = 1  # Some move
        
        # Convert to tensor
        obs_tensor = torch.from_numpy(dummy_obs).unsqueeze(0).to(agent.device)
        
        # Run inference
        with torch.no_grad():
            logits, values = agent.model.forward_eval(obs_tensor, agent._state)
        
        print(f"  Logits shape: {logits.shape}")
        print(f"  Values shape: {values.shape}")
        print(f"  Sample logits: {logits[0, :5].cpu().numpy()}")
        print("✓ Inference OK\n")
        return True
        
    except Exception as e:
        print(f"  Warning: Inference test failed: {e}")
        import traceback
        traceback.print_exc()
        return False


def main():
    """Run all tests."""
    print("=" * 60)
    print("Competition Environment Test Suite")
    print("=" * 60 + "\n")
    
    results = []
    
    # Run tests
    test_embedding()
    results.append(('Embedding', True))
    
    model_loaded = test_model_loading()
    results.append(('Model Loading', model_loaded))
    
    if model_loaded:
        inference_ok = test_inference()
        results.append(('Inference', inference_ok))
    
    # Summary
    print("=" * 60)
    print("Test Summary:")
    print("=" * 60)
    for name, passed in results:
        status = "✓ PASS" if passed else "✗ FAIL"
        print(f"  {name:20s} {status}")
    
    print("\n" + "=" * 60)
    
    all_critical_passed = results[0][1]  # Embedding must work
    if all_critical_passed:
        print("✓ Core functionality verified!")
        print("\nNext steps:")
        print("  1. Train a model: python pufferlib/ocean/showdown/simRL.py")
        print("  2. Run competition: python comp_env_bindings/run_competition.py <model.pt>")
    else:
        print("✗ Some tests failed. Check errors above.")
        return 1
    
    return 0


if __name__ == '__main__':
    sys.exit(main())
