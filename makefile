CC ?= gcc
INCLUDES ?= -I./sim_utils -I./data_sim -I./data_labels -I.

sim.out: 
	$(CC)  $(INCLUDES) sim.c -o sim.out

clean:
	rm -f *.out

debug_sim: clean 
	$(CC) -fdiagnostics-color=always $(INCLUDES) -g -DDEBUG sim.c -o debug_sim.out
