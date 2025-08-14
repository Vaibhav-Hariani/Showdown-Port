#include "battle.h"
#include "battle_queue.h"
#include "move_labels.h"
#include "pokedex_labels.h"

#include "pokemon.h"
#include "move.h"
#include "stdio.h"

battle b = {0};

// Update initialize_pokemon to use a local variable for pokemon_base[id - 1]
pokemon initialize_pokemon(POKEDEX id) {
    pokemon p;
    p.id = id;

    if (id < LAST_POKEMON) {
        const poke_ref *base = &pokemon_base[id - 1]; // Store reference to pokemon_base[id - 1]

        // Initialize stats from pokemon_base
        for (int i = 0; i < STAT_COUNT; i++) {
            p.stats.base_stats[i] = base->base_stats[i];
        }

        // Set Pokémon types
        p.type1 = base->primary_type;
        p.type2 = base->secondary_type;

        // Apply EVs/IVs logic for Gen1
        int iv = 15; // Max IV for Gen1 (0-15 scale)
        int ev = 65535; // Max EV for Gen1 (0-65535 scale)

        // Calculate HP stat separately
        p.stats.base_stats[STAT_HP] += (iv * 2 + ev / 4) / 100 + 10;

        // Calculate other stats
        for (int i = 1; i < STAT_COUNT; i++) {
            p.stats.base_stats[i] += (iv * 2 + ev / 4) / 100 + 5;
        }

        p.hp = p.stats.base_stats[STAT_HP];
        for (int i = 0; i < 4; i++) {
            p.poke_moves[i] = (i == 0) ? moves[TACKLE] : (move){0}; // Assign Tackle as the first move, others empty
        }
    }
    return p;
}


//ToDo: implement init_battle(), evaluate the queue
//Then, add checks for status effects, misses, etc. etc. Follow OU rules, and implement switching mechanics.
int main() {
    // Initialize battle
    battle b = {0};

    // Initialize two Pokémon dynamically
    pokemon bulbasaur1 = initialize_pokemon(BULBASAUR);
    pokemon bulbasaur2 = initialize_pokemon(BULBASAUR);

    // Assign Pokémon to players
    b.p1.team[0] = bulbasaur1;
    b.p2.team[0] = bulbasaur2;
    b.p1.active_pokemon = 0;
    b.p2.active_pokemon = 0;

    // Print initial state
    printf("Battle Start!\n");
    print_state(b.p1);
    print_state(b.p2);

    // Simplified battle loop
    while (b.p1.team[b.p1.active_pokemon].hp > 0 && b.p2.team[b.p2.active_pokemon].hp > 0) {
        step(&b, 0); // Call step function to handle a turn
    }

    // Determine winner
    if (b.p1.team[b.p1.active_pokemon].hp > 0) {
        printf("Player 1 wins!\n");
    } else {
        printf("Player 2 wins!\n");
    }

    return 0;
}
void print_state(player active) {
  for (int i = 0; i < 6; i++) {
    pokemon p = active.team[i];
    if (active.active_pokemon == i) {
      printf("Active Pokemon: \t");
    }
    printf("# %d: %s. HP: %d \n", i, PokemonNames[p.id], p.hp);
    // Designed for more details
    // print_poke_stats(p);
    printf("Available Moves: \n");
    for (int j = 0; j < 4; j++) {
      move m = p.poke_moves[j];
      printf("\t Label: %s; PP Remaining: %d \t", MoveLabels[m.id], m.pp);
      // Unsure if this function needs to exist
      // print_move_statline(m);
    }
  }
}

// Input value if valid, negative if selection failed
int make_move(battle* b, int active_player, player* p) {
  unsigned char input; 
  printf(
      "player %u move: "
      "(0-3 is move #)"
      "(4-9 is switch to pokemon #): ", active_player
      );
  scanf('%u', &input);
  // Input value if valid, negative if selection failed
  if (input > 9) {
    return -1;
  }
  action* cur = (b->action_queue.queue) + b->action_queue.q_size;
  b->action_queue.q_size++;
  if (input < 4) {
    move m = p->team[p->active_pokemon].poke_moves[input];
    if(m.pp <= 0) {
      return -1;
    }
    cur->action_type = move_action;
    cur->action_d.m = m;
    //According to battle_queue.h
    cur->order = 200;
  
  } else {
    input -= 4;
    if(p->team[input].hp <= 0){
    return -1;
    }
  cur->action_type = switch_action;
  cur->action_d.switch_target = input;
  cur->order = 103;
  }
  cur->p = p; 
  cur->player_num = active_player;
}

// Takes in a move from both players. then, resolves
void step(battle* b, int choice) {
  print_state(b->p1);
  printf("\n\n");
  print_state(b->p2);
  while (make_move(b, 1, &b->p1) > 0) {
    printf("\n Invalid selection from player %d: \n");
  }
  while (make_move(b, 2, &b->p2) > 0) {
    printf("\n Invalid selection from player %d: \n");
  }
  // Sort & evaluate the battlequeue on a move by move basis
  eval_queue(b);
}
