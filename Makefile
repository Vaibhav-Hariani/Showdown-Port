CC ?= clang
CFLAGS ?= -Wall -Wextra -Werror -O2 -std=c17
LDFLAGS ?= 
DEBUG ?= 0

all: showdown

showdown: a.out
	$(CC) $(CFLAGS) -Isim -Idata_sim -I data_labels sim/sim.c sim/battle.c sim/move.c sim/switch.c sim/battle_queue.c sim/log.c data_labels/*.c $(LDFLAGS)
