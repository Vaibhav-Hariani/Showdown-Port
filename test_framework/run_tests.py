#!/usr/bin/env python3
"""
Python wrapper for running Showdown-Port battle engine tests.
Provides additional analysis and visualization of test results.
"""

import subprocess
import sys
import csv
import json
from pathlib import Path
from typing import Dict, List, Tuple
import argparse


class TestResult:
    """Container for a single test result."""
    def __init__(self, suite: str, name: str, category: int, status: str,
                 expected_min: int, expected_max: int, actual: int,
                 deviation: float, error_msg: str):
        self.suite = suite
        self.name = name
        self.category = category
        self.status = status
        self.expected_min = expected_min
        self.expected_max = expected_max
        self.actual = actual
        self.deviation = deviation
        self.error_msg = error_msg

    @property
    def passed(self) -> bool:
        return self.status == "PASS"

    @property
    def failed(self) -> bool:
        return self.status == "FAIL"


class TestRunner:
    """Manages test execution and result analysis."""
    
    CATEGORY_NAMES = {
        0: "Damage Calculation",
        1: "Type Effectiveness",
        2: "Stat Modifiers",
        3: "Status Effects",
        4: "Critical Hits",
        5: "Accuracy",
        6: "STAB",
        7: "Weather",
        8: "Multi-Hit",
        9: "Integration"
    }

    def __init__(self, test_binary: str = "./test_battle", csv_file: str = "test_results.csv"):
        self.test_binary = test_binary
        self.csv_file = csv_file
        self.results: List[TestResult] = []

    def compile_tests(self) -> bool:
        """Compile the test suite."""
        print("🔨 Compiling test suite...")
        compile_cmd = [
            "gcc", "-o", "test_battle",
            "test_framework/main_test.c",
            "-I.", "-I./sim_utils", "-I./data_sim", "-I./test_framework",
            "-lm", "-g"
        ]
        
        try:
            result = subprocess.run(compile_cmd, capture_output=True, text=True, check=True)
            print("✓ Compilation successful")
            return True
        except subprocess.CalledProcessError as e:
            print("✗ Compilation failed:")
            print(e.stderr)
            return False

    def run_tests(self) -> Tuple[int, str]:
        """Execute the test binary and capture output."""
        print("\n🧪 Running tests...")
        try:
            result = subprocess.run(
                [self.test_binary],
                capture_output=True,
                text=True,
                check=False  # Don't raise on non-zero exit
            )
            print(result.stdout)
            if result.stderr:
                print("Errors:", result.stderr, file=sys.stderr)
            return result.returncode, result.stdout
        except FileNotFoundError:
            print(f"✗ Test binary not found: {self.test_binary}")
            print("  Run with --compile to build it first")
            return -1, ""
        except Exception as e:
            print(f"✗ Error running tests: {e}")
            return -1, ""

    def parse_results(self) -> bool:
        """Parse CSV results file."""
        csv_path = Path(self.csv_file)
        if not csv_path.exists():
            print(f"✗ Results file not found: {self.csv_file}")
            return False

        self.results = []
        try:
            with open(csv_path, 'r') as f:
                reader = csv.DictReader(f)
                for row in reader:
                    result = TestResult(
                        suite=row['Suite'],
                        name=row['Test Name'],
                        category=int(row['Category']),
                        status=row['Status'],
                        expected_min=int(row['Expected Min']),
                        expected_max=int(row['Expected Max']),
                        actual=int(row['Actual']),
                        deviation=float(row['Deviation %']),
                        error_msg=row['Error Message']
                    )
                    self.results.append(result)
            return True
        except Exception as e:
            print(f"✗ Error parsing results: {e}")
            return False

    def analyze_results(self):
        """Provide detailed analysis of test results."""
        if not self.results:
            print("No results to analyze")
            return

        print("\n" + "="*60)
        print("📊 DETAILED ANALYSIS")
        print("="*60)

        # Overall statistics
        total = len(self.results)
        passed = sum(1 for r in self.results if r.passed)
        failed = sum(1 for r in self.results if r.failed)
        pass_rate = (passed / total * 100) if total > 0 else 0

        print(f"\nOverall: {passed}/{total} tests passed ({pass_rate:.1f}%)")

        # By category
        print("\n📁 Results by Category:")
        categories = {}
        for result in self.results:
            cat = result.category
            if cat not in categories:
                categories[cat] = {"passed": 0, "failed": 0, "total": 0}
            categories[cat]["total"] += 1
            if result.passed:
                categories[cat]["passed"] += 1
            else:
                categories[cat]["failed"] += 1

        for cat_id, stats in sorted(categories.items()):
            cat_name = self.CATEGORY_NAMES.get(cat_id, f"Category {cat_id}")
            rate = (stats["passed"] / stats["total"] * 100) if stats["total"] > 0 else 0
            status = "✓" if stats["failed"] == 0 else "✗"
            print(f"  {status} {cat_name:25s} {stats['passed']:2d}/{stats['total']:2d} ({rate:5.1f}%)")

        # Failed tests
        failed_tests = [r for r in self.results if r.failed]
        if failed_tests:
            print(f"\n❌ Failed Tests ({len(failed_tests)}):")
            for result in failed_tests:
                print(f"\n  • {result.name}")
                print(f"    Expected: {result.expected_min}-{result.expected_max}")
                print(f"    Actual:   {result.actual}")
                print(f"    Error:    {result.error_msg}")

        # Deviation statistics
        deviations = [r.deviation for r in self.results if r.passed]
        if deviations:
            avg_dev = sum(deviations) / len(deviations)
            max_dev = max(deviations)
            print(f"\n📈 Deviation Statistics (passing tests):")
            print(f"  Average: {avg_dev:.2f}%")
            print(f"  Maximum: {max_dev:.2f}%")

    def generate_json_report(self, output_file: str = "test_results.json"):
        """Generate JSON report for programmatic consumption."""
        report = {
            "summary": {
                "total": len(self.results),
                "passed": sum(1 for r in self.results if r.passed),
                "failed": sum(1 for r in self.results if r.failed),
                "pass_rate": (sum(1 for r in self.results if r.passed) / len(self.results) * 100)
                            if self.results else 0
            },
            "tests": []
        }

        for result in self.results:
            report["tests"].append({
                "suite": result.suite,
                "name": result.name,
                "category": self.CATEGORY_NAMES.get(result.category, f"Category {result.category}"),
                "status": result.status,
                "expected_range": [result.expected_min, result.expected_max],
                "actual": result.actual,
                "deviation_percent": result.deviation,
                "error": result.error_msg
            })

        with open(output_file, 'w') as f:
            json.dump(report, f, indent=2)
        print(f"\n📄 JSON report saved to: {output_file}")

    def run_full_suite(self, compile_first: bool = False) -> int:
        """Run complete test pipeline."""
        if compile_first:
            if not self.compile_tests():
                return 1

        exit_code, _ = self.run_tests()
        
        if self.parse_results():
            self.analyze_results()
            self.generate_json_report()

        return exit_code


def main():
    parser = argparse.ArgumentParser(
        description="Run Showdown-Port battle engine tests",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Compile and run tests
  python3 run_tests.py --compile

  # Just run tests (binary already compiled)
  python3 run_tests.py

  # Analyze existing results without running
  python3 run_tests.py --analyze-only
        """
    )
    parser.add_argument(
        "--compile", "-c",
        action="store_true",
        help="Compile test suite before running"
    )
    parser.add_argument(
        "--analyze-only", "-a",
        action="store_true",
        help="Only analyze existing results (don't run tests)"
    )
    parser.add_argument(
        "--test-binary",
        default="./test_battle",
        help="Path to test binary (default: ./test_battle)"
    )
    parser.add_argument(
        "--csv-file",
        default="test_results.csv",
        help="Path to CSV results file (default: test_results.csv)"
    )

    args = parser.parse_args()

    runner = TestRunner(test_binary=args.test_binary, csv_file=args.csv_file)

    if args.analyze_only:
        if runner.parse_results():
            runner.analyze_results()
            runner.generate_json_report()
            return 0
        return 1

    exit_code = runner.run_full_suite(compile_first=args.compile)
    sys.exit(exit_code)


if __name__ == "__main__":
    main()
