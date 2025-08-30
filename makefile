CC ?= gcc
# CFLAGS ?= -Wall -Wextra -Werror
INCLUDES ?= -I./sim -I./data_sim -I./data_labels

sim.out: 
	$(CC) $(CFLAGS) $(INCLUDES) sim/sim.c -o sim.out
clean:
	rm -f sim.out

debug_sim: 
	$(CC) $(CFLAGS) $(INCLUDES) -g -DDEBUG sim/sim.c -o sim_debug.out
 