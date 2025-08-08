#include "battle.h"
#include "battle_queue.h"
#include "gamestate.h"
#include "move_labels.h"
#include "pokedex_labels.h"
#include "stdio.h"
battle b = {0};

int main() {}

void print_state(player active) {
  for (int i = 0; i < 6; i++) {
    pokemon p = active.team[i];
    if (active.active_pokemon == i) {
      printf("Active Pokemon: \t");
    }
    printf("# %d: %s. HP: %d \n", i, PokemonNames[p.id], p.hp);
    //Designed for more details
    print_poke_stats(p);
    printf("Available Moves: \n");
    for (int j = 0; j < 4; j++) {
      poke_moves m = p.moves[j];
      printf("\t Label: %s; PP Remaining: %d \t", MoveLabels[m.move_id], m.pp);
      // Unsure if this function needs to exist
      print_move_statline(m);
    }
  }
}

// Takes in a move from both players. then, resolves
void step(battle* b, int choice) {
  print_state(b->p1);
  printf("\n\n");
  print_state(b->p2);
  int input;
  printf(
      "player 1 action choice"
      "(0-3 is move #)"
      "(4-10 is switch to pokemon #): ");
  scanf('%d', &input);

  process_choice(b->p1, input);
  printf(
      "Player 2 action choice"
      "(0-3 is move #)"
      "(4-10 is switch to pokemon #): ");
  scanf('%d', &input);
  process_choice(b, b->p2, input);


}
