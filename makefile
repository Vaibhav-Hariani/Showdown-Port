CC ?= gcc
INCLUDES ?= -I./sim_utils -I./data_sim -I./data_labels -I.
TEST_INCLUDES ?= $(INCLUDES) -I./test_framework

sim.out: 
	$(CC)  $(INCLUDES) sim.c -o sim.out

clean:
	rm -f *.out test_battle test_results.csv test_results.json

debug_sim: clean 
	$(CC) -fdiagnostics-color=always $(INCLUDES) -g -DDEBUG sim.c -o debug_sim.out

# Test framework targets
test_battle: test_framework/main_test.c
	$(CC) -fdiagnostics-color=always $(TEST_INCLUDES) -g test_framework/main_test.c -o test_battle -lm

test: test_battle
	./test_battle

test_verbose: test_battle
	./test_battle -v

test_python: test_battle
	python3 test_framework/run_tests.py

test_clean:
	rm -f test_battle test_results.csv test_results.json

.PHONY: clean debug_sim test test_verbose test_python test_clean
