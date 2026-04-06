CC ?= gcc
INCLUDES ?= -I./sim_utils -I./data_sim -I./data_labels -I.
FLAGS ?= -Werror -O2

sim.out: 
	$(CC)  $(INCLUDES) $(FLAGS) sim.c -o sim.out

clean:
	rm -f *.out

debug_sim: clean 
	$(CC) -fdiagnostics-color=always $(INCLUDES) $(FLAGS) -g -DDEBUG sim.c -o debug_sim.out

testing: debug_sim
	$(CC) -fdiagnostics-color=always $(INCLUDES) $(FLAGS) -g -DDEBUG testing_framework/move_tests.c -o testing.out

damage_tests: debug_sim
	$(CC) -fdiagnostics-color=always $(INCLUDES) $(FLAGS) -g testing_framework/damage_tests.c -o damage_tests.out

generate_damage_dataset:
	/home/vaibh/Documents/Showdown-Port/.venv/bin/python testing_framework/generate_damage_dataset.py

run_damage_tests: damage_tests
	./damage_tests.out
