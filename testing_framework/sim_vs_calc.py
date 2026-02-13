"""
Compare a single-step Showdown simulator move with the official damage calc.

- Uses the Showdown Python env (unvectorized: 1 env, 2 agents).
- Parses the board with ShowdownParser from py_print.
- Picks a random available move for P1 and executes it.
- Computes expected damage via @smogon/calc (through damage_calc.DamageCalculator).
- Reports simulator-observed HP delta vs calc range.

Prereqs (run from testing_framework/):
  npm install
  pip install -r requirements.txt

Usage:
  pytest sim_vs_calc.py
"""

import os
import random
import numpy as np

from pufferlib.ocean.showdown.showdown import Showdown
from pufferlib.ocean.showdown.py_print import ShowdownParser
from damage_calc import DamageCalculator, PokemonData, MoveData


def assert_results(
    sim_damage_scaled,
    calc_damages,
    sim_status_after,
    sim_boosts_after,
    player_name,
    original_cur_hp,
    calc_result,
):
    actual_damage = round(sim_damage_scaled * original_cur_hp / 1000)
    assert (
        actual_damage in calc_damages
    ), f"{player_name}: Simulator dealt {actual_damage}, calculator expected one of {calc_damages}"

    # Check that no status effects were applied (calculator doesn't predict status changes)
    active_statuses = {k: v for k, v in sim_status_after.items() if v}
    assert (
        not active_statuses
    ), f"{player_name}: Unexpected status effects: {active_statuses}"

    # Check that boosts are neutral for both attacker and defender (calculator doesn't predict boost changes)
    # Simulator returns boosts offset by 6 (6 = neutral, 12 = +6, 0 = -6)
    neutral_boosts = {k: v - 6 for k, v in sim_boosts_after.items()}
    non_neutral = {k: v for k, v in neutral_boosts.items() if v != 0}
    assert not non_neutral, f"{player_name}: Unexpected boost changes: {non_neutral}"

    # Check that calculator input boosts match expected neutral state
    attacker_boosts = calc_result.attacker.boosts
    defender_boosts = calc_result.defender.boosts

    # Convert calculator boosts to zero-centered (they come as direct stage values)
    neutral_attacker_boosts = {k: v for k, v in attacker_boosts.items() if v != 0}
    neutral_defender_boosts = {k: v for k, v in defender_boosts.items() if v != 0}

    assert (
        not neutral_attacker_boosts
    ), f"{player_name}: Attacker had non-neutral boosts in calculator: {neutral_attacker_boosts}"
    assert (
        not neutral_defender_boosts
    ), f"{player_name}: Defender had non-neutral boosts in calculator: {neutral_defender_boosts}"


def choose_random_move(active_pokemon):
    moves = active_pokemon.get("moves") or []
    valid = [
        (i, m.get("name"))
        for i, m in enumerate(moves[:4])
        if m and m.get("name") and m.get("pp", 0) > 0
    ]
    return random.choice(valid) if valid else (0, None)


def test_simulator_vs_calculator():
    seed = random.randint(0, 2**31 - 1)
    env = Showdown(num_envs=1, num_agents=2, seed=seed)
    print(f"Using random seed: {seed}")
    ob, _ = env.reset(seed=seed)

    parsed_players = [ShowdownParser.parse_observation(ob) for ob in ob]
    p1 = parsed_players[0]["players"][0]
    p2 = parsed_players[1]["players"][1]
    p1_active = p1["active_pokemon"]
    p2_active = p2["active_pokemon"]

    move_idx, move_name = choose_random_move(p1_active)
    move_idx_p2, move_name_p2 = choose_random_move(p2_active)

    if move_name is None:
        print("No moves found for P1 active; aborting.")
        return
    if move_name_p2 is None:
        print("No moves found for P2 active; aborting.")
        return

    print(f"P1 selected move: {move_name} (action {move_idx})")
    print(f"P2 selected move: {move_name_p2} (action {move_idx_p2})")
    print(f"Attacker: {p1_active.get('name')}, Defender: {p2_active.get('name')}")

    hp_before_p1 = p1_active.get("hp_scaled")
    hp_before_p2 = p2_active.get("hp_scaled")
    status_before_p1 = p1_active.get("status", {})
    status_before_p2 = p2_active.get("status", {})
    boosts_before_p1 = p1_active.get("stat_mods", {})
    boosts_before_p2 = p2_active.get("stat_mods", {})

    # Convert move indices to simulator actions (moves are 6-9, switches are 0-5)
    actions = np.array([move_idx + 6, move_idx_p2 + 6], dtype=np.int64)
    ob2, *dump = env.step(actions)
    parsed_afters = [ShowdownParser.parse_observation(ob) for ob in ob2]

    p1_after = parsed_afters[0]["players"][0]
    p2_after = parsed_afters[1]["players"][1]

    hp_after_p1, status_after_p1, boosts_after_p1 = 0, {}, {}
    hp_after_p2, status_after_p2, boosts_after_p2 = 0, {}, {}

    if p1_after and p1_after["active_pokemon"]:
        p1_active_after = p1_after["active_pokemon"]
        hp_after_p1 = p1_active_after.get("hp_scaled", 0)
        status_after_p1 = p1_active_after.get("status", {})
        boosts_after_p1 = p1_active_after.get("stat_mods", {})
    if p2_after and p2_after["active_pokemon"]:
        p2_active_after = p2_after["active_pokemon"]
        hp_after_p2 = p2_active_after.get("hp_scaled", 0)
        status_after_p2 = p2_active_after.get("status", {})
        boosts_after_p2 = p2_active_after.get("stat_mods", {})

    p1_fainted = hp_after_p1 == 0
    p2_fainted = hp_after_p2 == 0

    sim_damage_p1 = hp_before_p1 - hp_after_p1
    sim_damage_p2 = hp_before_p2 - hp_after_p2

    print(f"\n=== SIMULATOR RESULTS ===")
    print(
        f"P1: HP {hp_before_p1}/1000 -> {hp_after_p1}/1000 (scaled damage: {sim_damage_p1})"
    )
    print(
        f"P2: HP {hp_before_p2}/1000 -> {hp_after_p2}/1000 (scaled damage: {sim_damage_p2})"
    )

    with DamageCalculator() as calc:
        full_evs = {
            "hp": 252,
            "atk": 252,
            "def": 252,
            "spa": 252,
            "spd": 252,
            "spe": 252,
        }
        full_ivs = {"hp": 31, "atk": 31, "def": 31, "spa": 31, "spd": 31, "spe": 31}

        attacker_p1 = PokemonData(
            name=p1_active["name"],
            evs=full_evs,
            ivs=full_ivs,
            boosts=boosts_before_p1,
            status=next((k for k, v in status_before_p1.items() if v), None),
        )
        defender_p2 = PokemonData(
            name=p2_active["name"],
            evs=full_evs,
            ivs=full_ivs,
            boosts=boosts_before_p2,
            status=next((k for k, v in status_before_p2.items() if v), None),
        )
        move_p1 = MoveData(name=move_name)
        result_p1 = calc.calculate(
            gen=1, attacker=attacker_p1, defender=defender_p2, move=move_p1
        )

        calc_damages_p2 = result_p1.damage
        print(f"\n=== CALCULATOR RESULTS (P1 -> P2) ===")
        print(f"Move: {move_name}")
        print(
            f"Damage range: {result_p1.min_damage()} - {result_p1.max_damage()} (avg {result_p1.average_damage():.1f})"
        )
        print(f"Defender boosts: {result_p1.defender['boosts'] or 'None'}")
        print(f"Description: {result_p1.description}")

        attacker_p2 = PokemonData(
            name=p2_active["name"],
            evs=full_evs,
            ivs=full_ivs,
            boosts=boosts_before_p2,
            status=next((k for k, v in status_before_p2.items() if v), None),
        )
        defender_p1 = PokemonData(
            name=p1_active["name"],
            evs=full_evs,
            ivs=full_ivs,
            boosts=boosts_before_p1,
            status=next((k for k, v in status_before_p1.items() if v), None),
        )
        move_p2 = MoveData(name=move_name_p2)
        result_p2 = calc.calculate(
            gen=1, attacker=attacker_p2, defender=defender_p1, move=move_p2
        )

        calc_damages_p1 = result_p2.damage

        print(f"\n=== CALCULATOR RESULTS (P2 -> P1) ===")
        print(f"Move: {move_name_p2}")
        print(
            f"Damage range: {result_p2.min_damage()} - {result_p2.max_damage()} (avg {result_p2.average_damage():.1f})"
        )
        print(f"Defender boosts: {result_p2.defender['boosts'] or 'None'}")
        print(f"Description: {result_p2.description}")

        print(f"\n=== ASSERTIONS ===")

        if not p2_fainted:
            assert_results(
                sim_damage_p2,
                calc_damages_p2,
                status_after_p2,
                boosts_after_p2,
                "P2 (from P1's attack)",
                result_p1.originalCurHP,
                result_p1,
            )
            print("P2: All assertions passed ✓")
        else:
            print("P2: Skipped (fainted, cannot verify P1's attack)")

        if not p1_fainted:
            assert_results(
                sim_damage_p1,
                calc_damages_p1,
                status_after_p1,
                boosts_after_p1,
                "P1 (from P2's attack)",
                result_p2.originalCurHP,
                result_p2,
            )
            print("P1: All assertions passed ✓")
        else:
            print("P1: Skipped (fainted, cannot verify P2's attack)")

    env.close()


if __name__ == "__main__":
    test_simulator_vs_calculator()
