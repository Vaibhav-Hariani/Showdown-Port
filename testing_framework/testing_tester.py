"""
Example usage of the Smogon damage calculator Python interface.

This script demonstrates how to use the damage calculator to compute
damage ranges for Pokemon battles.
"""

from damage_calc import DamageCalculator, PokemonData, MoveData, FieldData


def example_basic_calculation():
    """Basic damage calculation example."""
    print("=" * 70)
    print("Example 1: Basic Damage Calculation")
    print("=" * 70)
    
    # Create calculator instance
    calc = DamageCalculator()
    calc.start()
    
    try:
        # Gen 5 example: Gengar vs Chansey
        attacker = PokemonData(
            name="Gengar",
            item="Choice Specs",
            evs={"spa": 252, "spe": 252, "hp": 4},
            boosts={"spa": 1}
        )
        
        defender = PokemonData(
            name="Chansey",
            item="Eviolite",
            evs={"hp": 252, "spd": 252, "def": 4}
        )
        
        move = MoveData(name="Focus Blast")
        
        result = calc.calculate(gen=5, attacker=attacker, defender=defender, move=move)
        
        print(f"\n{result.description}")
        print(f"\nDamage: min={result.min_damage()}, max={result.max_damage()}, avg={result.average_damage():.1f}")
        print(f"Raw damage data: {result.damage}")
        
    finally:
        calc.stop()


def example_gen1_calculation():
    """Gen 1 example with critical hits."""
    print("\n" + "=" * 70)
    print("Example 2: Gen 1 Critical Hit Calculation")
    print("=" * 70)
    
    calc = DamageCalculator()
    calc.start()
    
    try:
        attacker = PokemonData(
            name="Tauros",
            level=100,
            nature="Adamant",
            evs={"atk": 252, "spe": 252}
        )
        
        defender = PokemonData(
            name="Chansey",
            level=100,
            evs={"hp": 252}
        )
        
        # Normal hit
        move = MoveData(name="Body Slam", isCrit=False)
        result_normal = calc.calculate(gen=1, attacker=attacker, defender=defender, move=move)
        
        # Critical hit
        move_crit = MoveData(name="Body Slam", isCrit=True)
        result_crit = calc.calculate(gen=1, attacker=attacker, defender=defender, move=move_crit)
        
        print(f"\nNormal Hit: {result_normal.description}")
        print(f"Damage: {result_normal.min_damage()}-{result_normal.max_damage()}")
        
        print(f"\nCritical Hit: {result_crit.description}")
        print(f"Damage: {result_crit.min_damage()}-{result_crit.max_damage()}")
        
    finally:
        calc.stop()


def example_weather_calculation():
    """Example with weather effects."""
    print("\n" + "=" * 70)
    print("Example 3: Weather Calculation")
    print("=" * 70)
    
    calc = DamageCalculator()
    calc.start()
    
    try:
        attacker = PokemonData(
            name="Kingdra",
            ability="Swift Swim",
            item="Life Orb",
            nature="Modest",
            evs={"spa": 252, "spe": 252, "hp": 4}
        )
        
        defender = PokemonData(
            name="Ferrothorn",
            ability="Iron Barbs",
            item="Leftovers",
            nature="Relaxed",
            evs={"hp": 252, "def": 168, "spd": 88}
        )
        
        move = MoveData(name="Hydro Pump")
        
        # Without rain
        result_no_rain = calc.calculate(gen=8, attacker=attacker, defender=defender, move=move)
        
        # With rain
        field = FieldData(weather="Rain")
        result_rain = calc.calculate(gen=8, attacker=attacker, defender=defender, move=move, field=field)
        
        print(f"\nNo Rain: {result_no_rain.description}")
        print(f"Damage: {result_no_rain.min_damage()}-{result_no_rain.max_damage()}")
        
        print(f"\nWith Rain: {result_rain.description}")
        print(f"Damage: {result_rain.min_damage()}-{result_rain.max_damage()}")
        
    finally:
        calc.stop()


def example_context_manager():
    """Example using context manager for automatic cleanup."""
    print("\n" + "=" * 70)
    print("Example 4: Using Context Manager")
    print("=" * 70)
    
    # Using 'with' statement ensures proper cleanup
    with DamageCalculator() as calc:
        attacker = PokemonData(
            name="Garchomp",
            ability="Rough Skin",
            item="Choice Scarf",
            nature="Jolly",
            evs={"atk": 252, "spe": 252, "hp": 4}
        )
        
        defender = PokemonData(
            name="Landorus-Therian",
            ability="Intimidate",
            item="Rocky Helmet",
            nature="Impish",
            evs={"hp": 252, "def": 252, "spd": 4}
        )
        
        move = MoveData(name="Earthquake")
        
        result = calc.calculate(gen=8, attacker=attacker, defender=defender, move=move)
        
        print(f"\n{result.description}")
        print(f"Damage: {result.min_damage()}-{result.max_damage()}")
        print(f"Avg: {result.average_damage():.1f}")


def example_stat_boosts():
    """Example with stat boosts."""
    print("\n" + "=" * 70)
    print("Example 5: Stat Boosts Calculation")
    print("=" * 70)
    
    with DamageCalculator() as calc:
        # +2 Attack Dragonite
        attacker = PokemonData(
            name="Dragonite",
            ability="Multiscale",
            item="Weakness Policy",
            nature="Adamant",
            evs={"atk": 252, "spe": 252, "hp": 4},
            boosts={"atk": 2}  # After Weakness Policy activation
        )
        
        defender = PokemonData(
            name="Ferrothorn",
            ability="Iron Barbs",
            item="Leftovers",
            nature="Relaxed",
            evs={"hp": 252, "def": 252, "spd": 4}
        )
        
        move = MoveData(name="Fire Punch")
        
        result = calc.calculate(gen=8, attacker=attacker, defender=defender, move=move)
        
        print(f"\n{result.description}")
        print(f"Damage: {result.min_damage()}-{result.max_damage()}")


def example_multi_hit():
    """Example with multi-hit moves."""
    print("\n" + "=" * 70)
    print("Example 6: Multi-Hit Move Calculation")
    print("=" * 70)
    
    with DamageCalculator() as calc:
        attacker = PokemonData(
            name="Cloyster",
            ability="Skill Link",
            item="Life Orb",
            nature="Jolly",
            evs={"atk": 252, "spe": 252, "hp": 4},
            boosts={"atk": 2}
        )
        
        defender = PokemonData(
            name="Volcarona",
            ability="Flame Body",
            item="Heavy-Duty Boots",
            nature="Timid",
            evs={"spa": 252, "spe": 252, "hp": 4}
        )
        
        # Icicle Spear hits 5 times with Skill Link
        move = MoveData(name="Icicle Spear", hits=5)
        
        result = calc.calculate(gen=8, attacker=attacker, defender=defender, move=move)
        
        print(f"\n{result.description}")
        print(f"Total Damage: {result.min_damage()}-{result.max_damage()}")


if __name__ == "__main__":
    print("\n" + "=" * 70)
    print("Smogon Damage Calculator - Python Interface Examples")
    print("=" * 70)
    print("\nMake sure you've run 'npm install' in the testing_framework directory!")
    print("This will install the @smogon/calc package.\n")
    
    try:
        example_basic_calculation()
        example_gen1_calculation()
        example_weather_calculation()
        example_context_manager()
        example_stat_boosts()
        example_multi_hit()
        
        print("\n" + "=" * 70)
        print("All examples completed successfully!")
        print("=" * 70)
        
    except Exception as e:
        print(f"\nError: {e}")
        print("\nMake sure:")
        print("1. Node.js is installed (version 14+)")
        print("2. You've run 'npm install' in testing_framework/")
        print("3. You've installed Python dependencies: pip install requests")
