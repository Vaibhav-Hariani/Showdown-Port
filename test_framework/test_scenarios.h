#ifndef TEST_SCENARIOS_H
#define TEST_SCENARIOS_H

#include "test_types.h"
#include "../data_sim/poke_enum.h"
#include "../data_sim/generated_move_enum.h"
#include "../sim_utils/poke_structs.h"

// Initialize default stat mods (all neutral)
static inline stat_mods default_stat_mods() {
    stat_mods mods = {0};
    mods.attack = 0;
    mods.defense = 0;
    mods.speed = 0;
    mods.specA = 0;
    mods.specD = 0;
    mods.accuracy = 0;
    mods.evasion = 0;
    return mods;
}

// Initialize default status (no status effects)
static inline struct STR_STATUS_FLAGS default_status() {
    struct STR_STATUS_FLAGS status = {0};
    return status;
}

// ============================================================================
// BASIC DAMAGE CALCULATION TESTS
// Based on Smogon Gen 1 damage calculator scenarios
// ============================================================================

// Test 1: Neutral damage, no modifiers
// Charizard Fire Blast vs Blastoise
// Expected: 90-106 (50.0 - 58.9%)
static inline TestCase test_neutral_special_attack() {
    TestCase test = {
        .test_name = "Neutral Special Attack",
        .description = "Charizard Fire Blast vs Blastoise - neutral type matchup",
        .category = TEST_CATEGORY_DAMAGE,
        .scenario = {
            .attacker_species = CHARIZARD,
            .attacker_level = 50,
            .attacker_hp = 153,
            .attacker_stats = {153, 104, 98, 129, 105, 120},  // Charizard stats at L50
            .attacker_stat_mods = default_stat_mods(),
            .attacker_status = default_status(),
            .attacker_type1 = FIRE,
            .attacker_type2 = FLYING,
            
            .defender_species = BLASTOISE,
            .defender_level = 50,
            .defender_hp = 158,
            .defender_stats = {158, 103, 120, 105, 125, 98},  // Blastoise stats at L50
            .defender_stat_mods = default_stat_mods(),
            .defender_status = default_status(),
            .defender_type1 = WATER,
            .defender_type2 = NONETYPE,
            
            .move_id = FIRE_BLAST_MOVE_ID,
            .move_power = 120,
            .move_accuracy = 0.85,
            .move_type = FIRE,
            .move_category = SPECIAL_MOVE_CATEGORY,
            
            .expected = {
                .min_damage = 90,
                .max_damage = 106,
                .type_effectiveness = 1.0,
                .is_critical = false,
                .stab_multiplier = 1.5
            }
        }
    };
    return test;
}

// Test 2: Super effective attack
// Starmie Thunderbolt vs Gyarados
// Expected: 200-236 (100% - guaranteed OHKO due to 4x weakness)
static inline TestCase test_super_effective_4x() {
    TestCase test = {
        .test_name = "4x Super Effective",
        .description = "Starmie Thunderbolt vs Gyarados - 4x weakness",
        .category = TEST_CATEGORY_TYPE_EFFECTIVENESS,
        .scenario = {
            .attacker_species = STARMIE,
            .attacker_level = 50,
            .attacker_hp = 155,
            .attacker_stats = {155, 95, 105, 120, 105, 135},
            .attacker_stat_mods = default_stat_mods(),
            .attacker_status = default_status(),
            .attacker_type1 = WATER,
            .attacker_type2 = PSYCHIC,
            
            .defender_species = GYARADOS,
            .defender_level = 50,
            .defender_hp = 170,
            .defender_stats = {170, 145, 99, 80, 120, 101},
            .defender_stat_mods = default_stat_mods(),
            .defender_status = default_status(),
            .defender_type1 = WATER,
            .defender_type2 = FLYING,
            
            .move_id = THUNDERBOLT_MOVE_ID,
            .move_power = 95,
            .move_accuracy = 1.0,
            .move_type = ELECTRIC,
            .move_category = SPECIAL_MOVE_CATEGORY,
            
            .expected = {
                .min_damage = 200,
                .max_damage = 236,
                .type_effectiveness = 4.0,  // 2x from Water, 2x from Flying
                .is_critical = false,
                .stab_multiplier = 1.0
            }
        }
    };
    return test;
}

// Test 3: Not very effective attack
// Alakazam Psychic vs Gengar
// Expected: 0 (immune due to Ghost type in Gen 1)
static inline TestCase test_immune_type() {
    TestCase test = {
        .test_name = "Type Immunity",
        .description = "Alakazam Psychic vs Gengar - Ghost immune to Psychic in Gen 1",
        .category = TEST_CATEGORY_TYPE_EFFECTIVENESS,
        .scenario = {
            .attacker_species = ALAKAZAM,
            .attacker_level = 50,
            .attacker_hp = 130,
            .attacker_stats = {130, 70, 65, 155, 105, 140},
            .attacker_stat_mods = default_stat_mods(),
            .attacker_status = default_status(),
            .attacker_type1 = PSYCHIC,
            .attacker_type2 = NONETYPE,
            
            .defender_species = GENGAR,
            .defender_level = 50,
            .defender_hp = 135,
            .defender_stats = {135, 85, 80, 150, 95, 130},
            .defender_stat_mods = default_stat_mods(),
            .defender_status = default_status(),
            .defender_type1 = GHOST,
            .defender_type2 = POISON,
            
            .move_id = PSYCHIC_MOVE_ID,
            .move_power = 90,
            .move_accuracy = 1.0,
            .move_type = PSYCHIC,
            .move_category = SPECIAL_MOVE_CATEGORY,
            
            .expected = {
                .min_damage = 0,
                .max_damage = 0,
                .type_effectiveness = 0.0,
                .is_critical = false,
                .stab_multiplier = 1.5
            }
        }
    };
    return test;
}

// Test 4: Physical attack with stat modifier
// Tauros Body Slam at +2 Attack vs Chansey
static inline TestCase test_stat_modifier_boost() {
    TestCase test = {
        .test_name = "Stat Boost (+2 Attack)",
        .description = "Tauros Body Slam with +2 Attack vs Chansey",
        .category = TEST_CATEGORY_STAT_MODIFIERS,
        .scenario = {
            .attacker_species = TAUROS,
            .attacker_level = 50,
            .attacker_hp = 150,
            .attacker_stats = {150, 120, 115, 60, 90, 130},
            .attacker_stat_mods = default_stat_mods(),
            .attacker_status = default_status(),
            .attacker_type1 = NORMAL,
            .attacker_type2 = NONETYPE,
            
            .defender_species = CHANSEY,
            .defender_level = 50,
            .defender_hp = 330,
            .defender_stats = {330, 25, 25, 55, 125, 70},
            .defender_stat_mods = default_stat_mods(),
            .defender_status = default_status(),
            .defender_type1 = NORMAL,
            .defender_type2 = NONETYPE,
            
            .move_id = BODY_SLAM_MOVE_ID,
            .move_power = 85,
            .move_accuracy = 1.0,
            .move_type = NORMAL,
            .move_category = PHYSICAL_MOVE_CATEGORY,
            
            .expected = {
                .min_damage = 170,
                .max_damage = 200,
                .type_effectiveness = 1.0,
                .is_critical = false,
                .stab_multiplier = 1.5
            }
        }
    };
    return test;
}

// Test 5: Burn effect halving physical damage
// Machamp Submission (Burned) vs Snorlax
static inline TestCase test_burn_physical_reduction() {
    TestCase test = {
        .test_name = "Burn Physical Reduction",
        .description = "Machamp Submission (Burned) vs Snorlax - burn halves physical attack",
        .category = TEST_CATEGORY_STATUS_EFFECTS,
        .scenario = {
            .attacker_species = MACHAMP,
            .attacker_level = 50,
            .attacker_hp = 165,
            .attacker_stats = {165, 150, 100, 85, 105, 75},
            .attacker_stat_mods = default_stat_mods(),
            .attacker_status = {.paralyzed = 0, .burn = 1, .freeze = 0, .poison = 0, .sleep = 0},
            .attacker_type1 = FIGHTING,
            .attacker_type2 = NONETYPE,
            
            .defender_species = SNORLAX,
            .defender_level = 50,
            .defender_hp = 235,
            .defender_stats = {235, 130, 85, 85, 130, 50},
            .defender_stat_mods = default_stat_mods(),
            .defender_status = default_status(),
            .defender_type1 = NORMAL,
            .defender_type2 = NONETYPE,
            
            .move_id = SUBMISSION_MOVE_ID,
            .move_power = 80,
            .move_accuracy = 0.80,
            .move_type = FIGHTING,
            .move_category = PHYSICAL_MOVE_CATEGORY,
            
            .expected = {
                .min_damage = 45,  // Approximately half of unburnedof damage
                .max_damage = 54,
                .type_effectiveness = 2.0,  // Fighting vs Normal
                .is_critical = false,
                .stab_multiplier = 1.5
            }
        }
    };
    return test;
}

// Test 6: STAB effectiveness
// Pikachu Thunderbolt vs Starmie (with STAB)
static inline TestCase test_stab_bonus() {
    TestCase test = {
        .test_name = "STAB Bonus",
        .description = "Pikachu Thunderbolt vs Starmie - with STAB bonus",
        .category = TEST_CATEGORY_STAB,
        .scenario = {
            .attacker_species = PIKACHU,
            .attacker_level = 50,
            .attacker_hp = 115,
            .attacker_stats = {115, 75, 50, 70, 70, 110},
            .attacker_stat_mods = default_stat_mods(),
            .attacker_status = default_status(),
            .attacker_type1 = ELECTRIC,
            .attacker_type2 = NONETYPE,
            
            .defender_species = STARMIE,
            .defender_level = 50,
            .defender_hp = 155,
            .defender_stats = {155, 95, 105, 120, 105, 135},
            .defender_stat_mods = default_stat_mods(),
            .defender_status = default_status(),
            .defender_type1 = WATER,
            .defender_type2 = PSYCHIC,
            
            .move_id = THUNDERBOLT_MOVE_ID,
            .move_power = 95,
            .move_accuracy = 1.0,
            .move_type = ELECTRIC,
            .move_category = SPECIAL_MOVE_CATEGORY,
            
            .expected = {
                .min_damage = 90,
                .max_damage = 106,
                .type_effectiveness = 2.0,
                .is_critical = false,
                .stab_multiplier = 1.5
            }
        }
    };
    return test;
}

// Test 7: Minimum damage (1 HP)
// Weak attack should always deal at least 1 damage
static inline TestCase test_minimum_damage() {
    TestCase test = {
        .test_name = "Minimum Damage",
        .description = "Weak attack should deal minimum 1 HP damage",
        .category = TEST_CATEGORY_DAMAGE,
        .scenario = {
            .attacker_species = MAGIKARP,
            .attacker_level = 5,
            .attacker_hp = 21,
            .attacker_stats = {21, 12, 67, 18, 22, 92},
            .attacker_stat_mods = default_stat_mods(),
            .attacker_status = default_status(),
            .attacker_type1 = WATER,
            .attacker_type2 = NONETYPE,
            
            .defender_species = ONIX,
            .defender_level = 50,
            .defender_hp = 115,
            .defender_stats = {115, 65, 200, 50, 65, 90},
            .defender_stat_mods = default_stat_mods(),
            .defender_status = default_status(),
            .defender_type1 = ROCK,
            .defender_type2 = GROUND,
            
            .move_id = TACKLE_MOVE_ID,
            .move_power = 35,
            .move_accuracy = 0.95,
            .move_type = NORMAL,
            .move_category = PHYSICAL_MOVE_CATEGORY,
            
            .expected = {
                .min_damage = 1,
                .max_damage = 1,
                .type_effectiveness = 1.0,
                .is_critical = false,
                .stab_multiplier = 1.0
            }
        }
    };
    return test;
}

#endif // TEST_SCENARIOS_H
