#include "battle.h"
#include "battle_queue.h"
#include "move_labels.h"
#include "pokedex_labels.h"

#include "pokemon.h"
#include "move.h"
#include "stdio.h"

battle b = {0};

int main() {

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
