CC ?= gcc
# CFLAGS ?= -Wall -Wextra -Werror
INCLUDES ?= -I./sim_utils -I./data_sim -I./data_labels -I.

sim.out: 
	$(CC) $(CFLAGS) $(INCLUDES) sim.c -o sim.out

clean:
	rm -f *.out

debug_sim: clean 
	$(CC) $(CFLAGS) $(INCLUDES) -g -DDEBUG sim.c -o debug_sim.out
