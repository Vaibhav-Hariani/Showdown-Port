# Showdown-Port Test Framework

A comprehensive testing framework for the Showdown-Port C battle engine, inspired by and validated against the [Smogon Damage Calculator](https://github.com/smogon/damage-calc).

## Overview

This test framework validates the core battle mechanics of the Showdown-Port simulator, focusing on Generation 1 (RBY) damage calculations, type effectiveness, stat modifiers, and status effects.

## Architecture

### Core Components

1. **test_types.h** - Core data structures for test framework
   - `TestCase`: Individual test definition
   - `TestScenario`: Battle scenario setup
   - `TestSuite`: Collection of related tests
   - `TestSummary`: Aggregated test results

2. **test_scenarios.h** - Pre-defined test cases based on Smogon calculations
   - Neutral damage tests
   - Type effectiveness (super effective, not very effective, immunities)
   - STAB bonuses
   - Stat modifiers
   - Status effects (burn, paralysis, etc.)
   - Edge cases (minimum damage, overflow, etc.)

3. **test_runner.h** - Test execution engine
   - Test case runner with isolated environment
   - Result validation with configurable tolerances
   - Colored terminal output
   - CSV report generation

4. **main_test.c** - Main test executable
   - Entry point for test suite execution
   - Summary reporting
   - Exit code handling for CI/CD integration

## Building and Running

### Prerequisites

```bash
# Ensure you have gcc installed
gcc --version

# From the Showdown root directory
cd /home/vaibh/Documents/PufferLib/pufferlib/ocean/Showdown
```

### Compile the Test Suite

```bash
# Create test framework build
gcc -o test_battle \
    test_framework/main_test.c \
    -I. \
    -I./sim_utils \
    -I./data_sim \
    -I./test_framework \
    -lm \
    -g
```

### Run Tests

```bash
# Run all tests
./test_battle

# View detailed CSV report
cat test_results.csv
```

## Test Categories

### 1. Damage Calculation Tests
- Basic damage formula validation
- Random factor verification (85-100% range)
- Minimum damage (1 HP) enforcement

### 2. Type Effectiveness Tests
- Super effective multipliers (2x, 4x)
- Not very effective multipliers (0.5x, 0.25x)
- Type immunities (0x)
- Gen 1-specific type interactions (e.g., Psychic immune to Ghost)

### 3. STAB (Same-Type Attack Bonus) Tests
- 1.5x multiplier when move type matches attacker type
- Dual-type Pokemon STAB interactions

### 4. Stat Modifier Tests
- Attack/Defense stage modifications (-6 to +6)
- Special Attack/Special Defense modifications
- Critical hit stat modifier bypass

### 5. Status Effect Tests
- Burn: 50% physical attack reduction
- Paralysis: Speed reduction (not implemented yet in damage calc)
- Sleep/Freeze: Move prevention
- Poison/Badly Poisoned: End-of-turn damage

### 6. Edge Cases
- Level 1 vs Level 100 battles
- Maximum/minimum stat scenarios
- Overflow prevention
- Zero power move handling

## Validation Against Smogon Calculator

All test scenarios are derived from the Smogon Damage Calculator with expected ranges calculated for Generation 1 mechanics. The framework includes:

- **Damage Formula**: `((2 * Level / 5 + 2) * Power * Attack / Defense) / 50 + 2) * STAB * TypeEffectiveness * Random`
- **Random Factor**: 217-255 / 255 (approximately 0.85 - 1.0)
- **Type Chart**: Gen 1 RBY type effectiveness matrix
- **Stat Modifiers**: Multipliers from -6 to +6 stages

## Adding New Tests

### Create a New Test Scenario

Add to `test_scenarios.h`:

```c
static inline TestCase test_your_new_test() {
    TestCase test = {
        .test_name = "Your Test Name",
        .description = "Detailed description of what's being tested",
        .category = TEST_CATEGORY_DAMAGE,
        .scenario = {
            .attacker_species = CHARIZARD_POKE_ID,
            .attacker_level = 50,
            // ... fill in other fields
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
```

### Register Test in Suite

Add to `main_test.c`:

```c
TestCase test_cases[] = {
    test_neutral_special_attack(),
    test_super_effective_4x(),
    // ... existing tests
    test_your_new_test(),  // Add your test here
};
```

## Interpreting Results

### Terminal Output

```
=== Running Test Suite: Core Damage Calculation Tests ===
  [1/7] Running: Neutral Special Attack ... ✓ PASS (deviation: 2.3%)
  [2/7] Running: 4x Super Effective ... ✓ PASS
  [3/7] Running: Type Immunity ... ✓ PASS
  [4/7] Running: Stat Boost (+2 Attack) ... ✗ FAIL
    Damage 185 outside expected range [170-200] (with 10% tolerance: [153-220])
    Expected: 170-200, Got: 185
```

### CSV Report Format

```csv
Suite,Test Name,Category,Status,Expected Min,Expected Max,Actual,Deviation %,Error Message
Core Damage,Neutral Special Attack,0,PASS,90,106,98,0.00,""
Core Damage,4x Super Effective,1,PASS,200,236,218,0.00,""
```

## Configuration

### Damage Tolerance

Adjust tolerance in `test_runner.h`:

```c
#define DAMAGE_TOLERANCE_PERCENT 10.0  // ±10% tolerance
```

### Random Seed

For reproducible tests, seed is set in `run_test_case()`:

```c
srand(42);  // Fixed seed for deterministic tests
```

## Integration with CI/CD

The test suite returns appropriate exit codes:
- `0`: All tests passed
- `1`: One or more tests failed or errored

### Example GitHub Actions Integration

```yaml
name: Battle Engine Tests

on: [push, pull_request]

jobs:
  test:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      - name: Build Test Suite
        run: |
          gcc -o test_battle test_framework/main_test.c -I. -I./sim_utils -I./data_sim -I./test_framework -lm
      - name: Run Tests
        run: ./test_battle
      - name: Upload Results
        if: always()
        uses: actions/upload-artifact@v2
        with:
          name: test-results
          path: test_results.csv
```

## Future Enhancements

- [ ] Critical hit testing with varying rates
- [ ] Multi-hit move testing (2-5 hits)
- [ ] Weather effect testing
- [ ] Badge boost testing
- [ ] Reflect/Light Screen testing
- [ ] Generation 2+ mechanics
- [ ] Performance benchmarking
- [ ] Fuzzing for edge cases
- [ ] Integration with Python test harness

## References

- [Smogon Damage Calculator](https://github.com/smogon/damage-calc) - Primary reference for expected values
- [Bulbapedia Damage Formula](https://bulbapedia.bulbagarden.net/wiki/Damage) - Gen 1 mechanics documentation
- [Pokémon Showdown Simulator](https://github.com/smogon/pokemon-showdown) - Reference implementation

## License

Part of the Showdown-Port project. See main repository for license details.
