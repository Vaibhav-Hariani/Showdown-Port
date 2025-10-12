#ifndef TEST_RUNNER_H
#define TEST_RUNNER_H

#include "test_types.h"
#include "test_scenarios.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "../sim_utils/log.h"
#include "../sim_utils/battle.h"
#include "../sim_utils/move.h"

// ANSI color codes for terminal output
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_BOLD    "\033[1m"

// Tolerance for damage calculations (percentage)
#define DAMAGE_TOLERANCE_PERCENT 10.0

// Helper function to setup a Pokemon from test scenario
static inline void setup_pokemon_from_scenario(Pokemon* poke, BattlePokemon* battle_poke, 
                                                uint16_t species, int level, int hp, 
                                                int stats[6], stat_mods mods, struct STR_STATUS_FLAGS status,
                                                TYPE type1, TYPE type2) {
    poke->id = species;
    poke->stats.level = level;
    poke->hp = hp;
    poke->stats.base_stats[STAT_HP] = stats[0];
    poke->stats.base_stats[STAT_ATTACK] = stats[1];
    poke->stats.base_stats[STAT_DEFENSE] = stats[2];
    poke->stats.base_stats[STAT_SPECIAL_ATTACK] = stats[3];
    poke->stats.base_stats[STAT_SPECIAL_DEFENSE] = stats[4];
    poke->stats.base_stats[STAT_SPEED] = stats[5];
    poke->type1 = type1;
    poke->type2 = type2;
    poke->status = status;
    
    if (battle_poke) {
        battle_poke->pokemon = poke;
        battle_poke->stat_mods = mods;
        battle_poke->type1 = type1;
        battle_poke->type2 = type2;
        battle_poke->confusion_counter = 0;
        battle_poke->recharge_counter = 0;
        battle_poke->flinch = 0;
        battle_poke->sleep_ctr = 0;
    }
}

// Helper function to setup a Move from test scenario
static inline void setup_move_from_scenario(Move* move, uint16_t move_id, int power, 
                                             float accuracy, TYPE type, MOVE_CATEGORY category) {
    move->id = move_id;
    move->power = power;
    move->accuracy = accuracy;
    move->type = type;
    move->category = category;
    move->pp = 10;  // Default PP for testing
}

// Run a single test case
static inline void run_test_case(TestCase* test) {
    // Set seed for reproducibility within test
    srand(42);
    
    // Setup Pokemon
    Pokemon attacker_poke = {0};
    Pokemon defender_poke = {0};
    BattlePokemon attacker_battle = {0};
    BattlePokemon defender_battle = {0};
    
    TestScenario* s = &test->scenario;
    
    setup_pokemon_from_scenario(&attacker_poke, &attacker_battle,
                                  s->attacker_species, s->attacker_level, s->attacker_hp,
                                  s->attacker_stats, s->attacker_stat_mods, s->attacker_status,
                                  s->attacker_type1, s->attacker_type2);
    
    setup_pokemon_from_scenario(&defender_poke, &defender_battle,
                                  s->defender_species, s->defender_level, s->defender_hp,
                                  s->defender_stats, s->defender_stat_mods, s->defender_status,
                                  s->defender_type1, s->defender_type2);
    
    // Setup move
    Move test_move = {0};
    setup_move_from_scenario(&test_move, s->move_id, s->move_power, 
                             s->move_accuracy, s->move_type, s->move_category);
    
    // Calculate damage
    int damage = calculate_damage(&attacker_battle, &defender_battle, &test_move);
    
    test->actual_damage = damage;
    
    // Check if damage is within expected range
    int min_expected = s->expected.min_damage;
    int max_expected = s->expected.max_damage;
    
    // For immune/resisted moves, expect 0 damage
    if (s->expected.type_effectiveness == 0.0) {
        if (damage == 0) {
            test->status = TEST_PASS;
            test->damage_deviation_percent = 0.0;
            strcpy(test->error_message, "");
        } else {
            test->status = TEST_FAIL;
            test->damage_deviation_percent = 100.0;
            snprintf(test->error_message, sizeof(test->error_message),
                    "Expected 0 damage (immunity), got %d", damage);
        }
        return;
    }
    
    // Apply tolerance for damage range checks
    float tolerance = DAMAGE_TOLERANCE_PERCENT / 100.0;
    int min_with_tolerance = (int)(min_expected * (1.0 - tolerance));
    int max_with_tolerance = (int)(max_expected * (1.0 + tolerance));
    
    if (damage >= min_with_tolerance && damage <= max_with_tolerance) {
        test->status = TEST_PASS;
        // Calculate how close we are to expected range
        int mid_expected = (min_expected + max_expected) / 2;
        test->damage_deviation_percent = fabs(((float)damage - mid_expected) / mid_expected * 100.0);
        strcpy(test->error_message, "");
    } else {
        test->status = TEST_FAIL;
        int mid_expected = (min_expected + max_expected) / 2;
        test->damage_deviation_percent = fabs(((float)damage - mid_expected) / mid_expected * 100.0);
        snprintf(test->error_message, sizeof(test->error_message),
                "Damage %d outside expected range [%d-%d] (with %d%% tolerance: [%d-%d])",
                damage, min_expected, max_expected, (int)DAMAGE_TOLERANCE_PERCENT,
                min_with_tolerance, max_with_tolerance);
    }
}

// Run a test suite
static inline void run_test_suite(TestSuite* suite) {
    suite->passed = 0;
    suite->failed = 0;
    suite->skipped = 0;
    suite->errors = 0;
    
    printf("\n%s=== Running Test Suite: %s ===%s\n", COLOR_BOLD, suite->suite_name, COLOR_RESET);
    
    for (int i = 0; i < suite->test_count; i++) {
        TestCase* test = &suite->tests[i];
        printf("  [%d/%d] Running: %s ... ", i + 1, suite->test_count, test->test_name);
        fflush(stdout);
        
        run_test_case(test);
        
        switch (test->status) {
            case TEST_PASS:
                printf("%s✓ PASS%s", COLOR_GREEN, COLOR_RESET);
                if (test->damage_deviation_percent > 0.1) {
                    printf(" (deviation: %.1f%%)", test->damage_deviation_percent);
                }
                printf("\n");
                suite->passed++;
                break;
            case TEST_FAIL:
                printf("%s✗ FAIL%s\n", COLOR_RED, COLOR_RESET);
                printf("    %s\n", test->error_message);
                printf("    Expected: %d-%d, Got: %d\n", 
                       test->scenario.expected.min_damage,
                       test->scenario.expected.max_damage,
                       test->actual_damage);
                suite->failed++;
                break;
            case TEST_SKIP:
                printf("%s⊘ SKIP%s\n", COLOR_YELLOW, COLOR_RESET);
                suite->skipped++;
                break;
            case TEST_ERROR:
                printf("%s✗ ERROR%s\n", COLOR_RED, COLOR_RESET);
                printf("    %s\n", test->error_message);
                suite->errors++;
                break;
        }
    }
}

// Print test summary
static inline void print_test_summary(TestSummary* summary) {
    printf("\n%s%s========================================%s\n", 
           COLOR_BOLD, COLOR_CYAN, COLOR_RESET);
    printf("%s%sTEST SUMMARY%s\n", COLOR_BOLD, COLOR_CYAN, COLOR_RESET);
    printf("%s%s========================================%s\n", 
           COLOR_BOLD, COLOR_CYAN, COLOR_RESET);
    
    printf("Total Tests:  %d\n", summary->total_tests);
    printf("%sPassed:       %d%s\n", COLOR_GREEN, summary->total_passed, COLOR_RESET);
    
    if (summary->total_failed > 0) {
        printf("%sFailed:       %d%s\n", COLOR_RED, summary->total_failed, COLOR_RESET);
    } else {
        printf("Failed:       %d\n", summary->total_failed);
    }
    
    if (summary->total_skipped > 0) {
        printf("%sSkipped:      %d%s\n", COLOR_YELLOW, summary->total_skipped, COLOR_RESET);
    } else {
        printf("Skipped:      %d\n", summary->total_skipped);
    }
    
    if (summary->total_errors > 0) {
        printf("%sErrors:       %d%s\n", COLOR_RED, summary->total_errors, COLOR_RESET);
    } else {
        printf("Errors:       %d\n", summary->total_errors);
    }
    
    printf("Pass Rate:    %.1f%%\n", summary->pass_rate);
    printf("Execution:    %.2f ms\n", summary->execution_time_ms);
    
    printf("%s%s========================================%s\n\n", 
           COLOR_BOLD, COLOR_CYAN, COLOR_RESET);
    
    if (summary->total_failed == 0 && summary->total_errors == 0) {
        printf("%s%s🎉 All tests passed!%s\n", COLOR_BOLD, COLOR_GREEN, COLOR_RESET);
    } else {
        printf("%s%s⚠ Some tests failed. Review the output above.%s\n", 
               COLOR_BOLD, COLOR_RED, COLOR_RESET);
    }
}

// Generate detailed test report (CSV format)
static inline void generate_csv_report(TestSuite* suites, int suite_count, const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (!fp) {
        fprintf(stderr, "Error: Could not create report file %s\n", filename);
        return;
    }
    
    // CSV header
    fprintf(fp, "Suite,Test Name,Category,Status,Expected Min,Expected Max,Actual,Deviation %%,Error Message\n");
    
    for (int i = 0; i < suite_count; i++) {
        TestSuite* suite = &suites[i];
        for (int j = 0; j < suite->test_count; j++) {
            TestCase* test = &suite->tests[j];
            
            const char* status_str;
            switch (test->status) {
                case TEST_PASS: status_str = "PASS"; break;
                case TEST_FAIL: status_str = "FAIL"; break;
                case TEST_SKIP: status_str = "SKIP"; break;
                case TEST_ERROR: status_str = "ERROR"; break;
                default: status_str = "UNKNOWN";
            }
            
            fprintf(fp, "%s,%s,%d,%s,%d,%d,%d,%.2f,\"%s\"\n",
                   suite->suite_name,
                   test->test_name,
                   test->category,
                   status_str,
                   test->scenario.expected.min_damage,
                   test->scenario.expected.max_damage,
                   test->actual_damage,
                   test->damage_deviation_percent,
                   test->error_message);
        }
    }
    
    fclose(fp);
    printf("Detailed report saved to: %s\n", filename);
}

#endif // TEST_RUNNER_H
