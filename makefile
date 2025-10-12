CC ?= gcc
INCLUDES ?= -I./sim_utils -I./data_sim -I./data_labels -I.
FLAGS ?= -Werror -O2

sim.out: 
	$(CC)  $(INCLUDES) $(FLAGS) sim.c -o sim.out

clean:
	rm -f *.out

debug_sim: clean 
	$(CC) -fdiagnostics-color=always $(INCLUDES) $(FLAGS) -g -DDEBUG sim.c -o debug_sim.out
