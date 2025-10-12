#ifndef TEST_TYPES_H
#define TEST_TYPES_H

#include <stdint.h>
#include <stdbool.h>

// Forward declarations to avoid circular dependencies
#include "../sim_utils/poke_structs.h"
#include "../sim_utils/move_structs.h"
#include "../data_sim/typing.h"

// Test result status codes
typedef enum {
    TEST_PASS,
    TEST_FAIL,
    TEST_SKIP,
    TEST_ERROR
} TestStatus;

// Test category for organizing tests
typedef enum {
    TEST_CATEGORY_DAMAGE,
    TEST_CATEGORY_TYPE_EFFECTIVENESS,
    TEST_CATEGORY_STAT_MODIFIERS,
    TEST_CATEGORY_STATUS_EFFECTS,
    TEST_CATEGORY_CRITICAL_HITS,
    TEST_CATEGORY_ACCURACY,
    TEST_CATEGORY_STAB,
    TEST_CATEGORY_WEATHER,
    TEST_CATEGORY_MULTI_HIT,
    TEST_CATEGORY_INTEGRATION
} TestCategory;

// Expected damage range (min/max from Smogon calc)
typedef struct {
    int min_damage;
    int max_damage;
    float type_effectiveness;
    bool is_critical;
    float stab_multiplier;
} ExpectedDamage;

// Battle scenario setup
typedef struct {
    // Attacker setup
    uint16_t attacker_species;
    int attacker_level;
    int attacker_hp;
    int attacker_stats[6];  // HP, ATK, DEF, SPA, SPD, SPE
    stat_mods attacker_stat_mods;
    struct STR_STATUS_FLAGS attacker_status;
    TYPE attacker_type1;
    TYPE attacker_type2;
    
    // Defender setup
    uint16_t defender_species;
    int defender_level;
    int defender_hp;
    int defender_stats[6];
    stat_mods defender_stat_mods;
    struct STR_STATUS_FLAGS defender_status;
    TYPE defender_type1;
    TYPE defender_type2;
    
    // Move details
    uint16_t move_id;
    int move_power;
    float move_accuracy;
    TYPE move_type;
    MOVE_CATEGORY move_category;
    
    // Field conditions
    bool sun_active;
    bool rain_active;
    bool sandstorm_active;
    
    // Expected results
    ExpectedDamage expected;
} TestScenario;

// Individual test case
typedef struct {
    const char* test_name;
    const char* description;
    TestCategory category;
    TestScenario scenario;
    TestStatus status;
    char error_message[256];
    int actual_damage;
    float damage_deviation_percent;
} TestCase;

// Test suite containing multiple test cases
typedef struct {
    const char* suite_name;
    TestCase* tests;
    int test_count;
    int passed;
    int failed;
    int skipped;
    int errors;
} TestSuite;

// Test result summary
typedef struct {
    int total_tests;
    int total_passed;
    int total_failed;
    int total_skipped;
    int total_errors;
    float pass_rate;
    double execution_time_ms;
} TestSummary;

#endif // TEST_TYPES_H
