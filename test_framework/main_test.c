#include "test_runner.h"
#include "test_scenarios.h"
#include <time.h>

int main(int argc, char** argv) {
    printf("%s%s", COLOR_BOLD, COLOR_CYAN);
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║         Showdown-Port Battle Engine Test Suite            ║\n");
    printf("║    Based on Smogon Damage Calculator (Gen 1 RBY)          ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    printf("%s", COLOR_RESET);
    
    // Initialize test cases
    TestCase test_cases[] = {
        test_neutral_special_attack(),
        test_super_effective_4x(),
        test_immune_type(),
        test_stat_modifier_boost(),
        test_burn_physical_reduction(),
        test_stab_bonus(),
        test_minimum_damage()
    };
    
    int test_count = sizeof(test_cases) / sizeof(TestCase);
    
    // Create test suite
    TestSuite suite = {
        .suite_name = "Core Damage Calculation Tests",
        .tests = test_cases,
        .test_count = test_count,
        .passed = 0,
        .failed = 0,
        .skipped = 0,
        .errors = 0
    };
    
    // Track execution time
    clock_t start = clock();
    
    // Run the test suite
    run_test_suite(&suite);
    
    clock_t end = clock();
    double execution_time_ms = ((double)(end - start) / CLOCKS_PER_SEC) * 1000.0;
    
    // Generate summary
    TestSummary summary = {
        .total_tests = suite.test_count,
        .total_passed = suite.passed,
        .total_failed = suite.failed,
        .total_skipped = suite.skipped,
        .total_errors = suite.errors,
        .pass_rate = (suite.test_count > 0) ? 
                     ((float)suite.passed / suite.test_count * 100.0) : 0.0,
        .execution_time_ms = execution_time_ms
    };
    
    // Print summary
    print_test_summary(&summary);
    
    // Generate CSV report
    TestSuite suites[] = {suite};
    generate_csv_report(suites, 1, "test_results.csv");
    
    // Return appropriate exit code
    return (suite.failed > 0 || suite.errors > 0) ? 1 : 0;
}
