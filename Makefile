CC ?= clang
CFLAGS ?= -Wall -Wextra -Werror -O2 -std=c17
LDFLAGS ?= 
DEBUG ?= 0
EXTRA_FLAGS ?= 


sim.out: 
	$(CC) $(CFLAGS) $(EXTRA_FLAGS) -o sim.out -Isim -Idata_sim -I data_labels sim/sim.c sim/battle.c sim/move.c sim/switch.c sim/battle_queue.c sim/log.c data_labels/*.c $(LDFLAGS)

clean:
	rm -f sim.out
