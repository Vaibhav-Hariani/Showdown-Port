#!/usr/bin/env python3
"""
Test case generator using Smogon damage calculator API or local calculations.
This script helps generate comprehensive test scenarios programmatically.
"""

import json
from typing import Dict, List, Optional, Tuple
from dataclasses import dataclass, asdict


@dataclass
class PokemonData:
    """Pokemon data for test generation."""
    species_id: int
    name: str
    level: int
    hp: int
    stats: List[int]  # [HP, ATK, DEF, SPA, SPD, SPE]
    types: Tuple[str, Optional[str]]


@dataclass
class MoveData:
    """Move data for test generation."""
    move_id: int
    name: str
    power: int
    accuracy: float
    type: str
    category: str  # "Physical" or "Special"


@dataclass
class TestScenarioData:
    """Test scenario for C code generation."""
    test_name: str
    description: str
    attacker: PokemonData
    defender: PokemonData
    move: MoveData
    expected_min: int
    expected_max: int
    type_effectiveness: float
    stab: float


class TestGenerator:
    """Generate test cases for various battle scenarios."""
    
    # Gen 1 type chart
    TYPE_CHART = {
        "Normal": {"Rock": 0.5, "Ghost": 0.0},
        "Fire": {"Fire": 0.5, "Water": 0.5, "Grass": 2.0, "Ice": 2.0, "Bug": 2.0, "Rock": 0.5, "Dragon": 0.5},
        "Water": {"Fire": 2.0, "Water": 0.5, "Grass": 0.5, "Ground": 2.0, "Rock": 2.0, "Dragon": 0.5},
        "Electric": {"Water": 2.0, "Electric": 0.5, "Grass": 0.5, "Ground": 0.0, "Flying": 2.0, "Dragon": 0.5},
        "Grass": {"Fire": 0.5, "Water": 2.0, "Grass": 0.5, "Poison": 0.5, "Ground": 2.0, "Flying": 0.5, "Bug": 0.5, "Rock": 2.0, "Dragon": 0.5},
        "Ice": {"Fire": 0.5, "Water": 0.5, "Grass": 2.0, "Ice": 0.5, "Ground": 2.0, "Flying": 2.0, "Dragon": 2.0},
        "Fighting": {"Normal": 2.0, "Ice": 2.0, "Poison": 0.5, "Flying": 0.5, "Psychic": 0.5, "Bug": 0.5, "Rock": 2.0, "Ghost": 0.0},
        "Poison": {"Grass": 2.0, "Poison": 0.5, "Ground": 0.5, "Bug": 2.0, "Rock": 0.5, "Ghost": 0.5},
        "Ground": {"Fire": 2.0, "Electric": 2.0, "Grass": 0.5, "Poison": 2.0, "Flying": 0.0, "Bug": 0.5, "Rock": 2.0},
        "Flying": {"Electric": 0.5, "Grass": 2.0, "Fighting": 2.0, "Bug": 2.0, "Rock": 0.5},
        "Psychic": {"Fighting": 2.0, "Poison": 2.0, "Psychic": 0.5, "Ghost": 0.0},
        "Bug": {"Fire": 0.5, "Grass": 2.0, "Fighting": 0.5, "Poison": 2.0, "Flying": 0.5, "Psychic": 2.0, "Ghost": 0.5, "Rock": 0.5},
        "Rock": {"Fire": 2.0, "Ice": 2.0, "Fighting": 0.5, "Ground": 0.5, "Flying": 2.0, "Bug": 2.0},
        "Ghost": {"Normal": 0.0, "Psychic": 0.0, "Ghost": 2.0},
        "Dragon": {"Dragon": 2.0},
    }
    
    def calculate_type_effectiveness(self, move_type: str, defender_type1: str, 
                                     defender_type2: Optional[str]) -> float:
        """Calculate type effectiveness multiplier."""
        eff = 1.0
        
        if move_type in self.TYPE_CHART:
            if defender_type1 in self.TYPE_CHART[move_type]:
                eff *= self.TYPE_CHART[move_type][defender_type1]
            
            if defender_type2 and defender_type2 in self.TYPE_CHART[move_type]:
                eff *= self.TYPE_CHART[move_type][defender_type2]
        
        return eff
    
    def calculate_damage_range(self, attacker: PokemonData, defender: PokemonData, 
                                move: MoveData) -> Tuple[int, int]:
        """
        Calculate damage range using Gen 1 damage formula.
        Formula: ((2 * Level / 5 + 2) * Power * Attack / Defense) / 50 + 2) * STAB * Type * Random
        Random factor: 217-255 / 255.0 (approximately 0.85-1.0)
        """
        level = attacker.level
        power = move.power
        
        # Determine attack and defense stats
        if move.category == "Special":
            attack = attacker.stats[3]  # Special Attack
            defense = defender.stats[4]  # Special Defense
        else:  # Physical
            attack = attacker.stats[1]  # Attack
            defense = defender.stats[2]  # Defense
        
        # Base damage
        base = ((2 * level / 5 + 2) * power * attack / defense) / 50 + 2
        
        # STAB
        stab = 1.5 if move.type in attacker.types else 1.0
        
        # Type effectiveness
        type_eff = self.calculate_type_effectiveness(
            move.type, defender.types[0], defender.types[1]
        )
        
        # Apply modifiers
        damage = base * stab * type_eff
        
        # Random factor range
        min_random = 217 / 255.0
        max_random = 255 / 255.0
        
        min_damage = int(damage * min_random)
        max_damage = int(damage * max_random)
        
        return max(1, min_damage), max(1, max_damage)
    
    def generate_c_test_function(self, scenario: TestScenarioData) -> str:
        """Generate C code for a test scenario."""
        func_name = scenario.test_name.lower().replace(" ", "_").replace("-", "_")
        
        c_code = f"""
// Test: {scenario.test_name}
// {scenario.description}
static inline TestCase {func_name}() {{
    TestCase test = {{
        .test_name = "{scenario.test_name}",
        .description = "{scenario.description}",
        .category = TEST_CATEGORY_DAMAGE,
        .scenario = {{
            .attacker_species = {scenario.attacker.species_id},
            .attacker_level = {scenario.attacker.level},
            .attacker_hp = {scenario.attacker.hp},
            .attacker_stats = {{{', '.join(map(str, scenario.attacker.stats))}}},
            .attacker_stat_mods = default_stat_mods(),
            .attacker_status = default_status(),
            .attacker_type1 = {scenario.attacker.types[0].upper()},
            .attacker_type2 = {(scenario.attacker.types[1] or 'NONETYPE').upper()},
            
            .defender_species = {scenario.defender.species_id},
            .defender_level = {scenario.defender.level},
            .defender_hp = {scenario.defender.hp},
            .defender_stats = {{{', '.join(map(str, scenario.defender.stats))}}},
            .defender_stat_mods = default_stat_mods(),
            .defender_status = default_status(),
            .defender_type1 = {scenario.defender.types[0].upper()},
            .defender_type2 = {(scenario.defender.types[1] or 'NONETYPE').upper()},
            
            .move_id = {scenario.move.move_id},
            .move_power = {scenario.move.power},
            .move_accuracy = {scenario.move.accuracy},
            .move_type = {scenario.move.type.upper()},
            .move_category = {scenario.move.category.upper()}_MOVE_CATEGORY,
            
            .expected = {{
                .min_damage = {scenario.expected_min},
                .max_damage = {scenario.expected_max},
                .type_effectiveness = {scenario.type_effectiveness},
                .is_critical = false,
                .stab_multiplier = {scenario.stab}
            }}
        }}
    }};
    return test;
}}
"""
        return c_code
    
    def generate_test_suite(self, scenarios: List[TestScenarioData], 
                           output_file: str = "generated_tests.h"):
        """Generate a complete test suite header file."""
        header = """#ifndef GENERATED_TESTS_H
#define GENERATED_TESTS_H

#include "test_types.h"
#include "test_scenarios.h"

// Auto-generated test scenarios
// Generated by test_generator.py

"""
        
        for scenario in scenarios:
            header += self.generate_c_test_function(scenario)
        
        header += "\n#endif // GENERATED_TESTS_H\n"
        
        with open(output_file, 'w') as f:
            f.write(header)
        
        print(f"Generated {len(scenarios)} test scenarios in {output_file}")


def main():
    """Generate sample test cases."""
    generator = TestGenerator()
    
    # Example: Generate a few test scenarios
    scenarios = []
    
    # You can add more scenarios here based on Smogon calculations
    # or manual verification
    
    # Example scenario (you would populate these with actual data)
    # scenarios.append(TestScenarioData(...))
    
    if scenarios:
        generator.generate_test_suite(scenarios)
        print(f"✓ Generated {len(scenarios)} test scenarios")
    else:
        print("⚠ No scenarios defined. Add scenarios to generate tests.")
        print("  See test_scenarios.h for examples of manually defined tests.")


if __name__ == "__main__":
    main()
