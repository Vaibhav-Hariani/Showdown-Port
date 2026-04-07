CC ?= gcc
INCLUDES ?= -I./sim_utils -I./data_sim -I./data_labels -I.
COMMON_FLAGS ?= -Werror
RELEASE_FLAGS ?= -O3 -march=native -flto
DEBUG_FLAGS ?= -O2

sim.out: 
	$(CC) $(INCLUDES) $(COMMON_FLAGS) $(RELEASE_FLAGS) sim.c -o sim.out

profile_sim.out:
	$(CC) $(INCLUDES) $(COMMON_FLAGS) $(RELEASE_FLAGS) testing/profile_sim.c -o profile_sim.out

clean:
	rm -f *.out

debug_sim: clean 
	$(CC) -fdiagnostics-color=always $(INCLUDES) $(COMMON_FLAGS) $(DEBUG_FLAGS) -g -DDEBUG sim.c -o debug_sim.out

testing: debug_sim
	$(CC) -fdiagnostics-color=always $(INCLUDES) $(COMMON_FLAGS) $(DEBUG_FLAGS) -g -DDEBUG testing/move_tests.c -o testing.out

damage_tests: debug_sim
	$(CC) -fdiagnostics-color=always $(INCLUDES) $(COMMON_FLAGS) $(DEBUG_FLAGS) -g testing/damage_tests.c -o damage_tests.out

generate_damage_dataset:
	/home/vaibh/Documents/Showdown-Port/.venv/bin/python testing/generate_damage_dataset.py

run_damage_tests: damage_tests
	./damage_tests.out

run_profile_sim: profile_sim.out
	./profile_sim.out 1000 3
