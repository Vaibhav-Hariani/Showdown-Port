CC ?= gcc
# CFLAGS ?= -Wall -Wextra -Werror
INCLUDES ?= -I./sim -I./data_sim -I./data_labels

sim.out: 
	$(CC) $(CFLAGS) $(INCLUDES) sim/sim.c -o sim.out

clean:
	rm -f *.out
debug_sim: clean 
	$(CC) $(CFLAGS) $(INCLUDES) -g -DDEBUG sim/sim.c -o debug_sim.out
