#include "sim.h"


// Convert to obs of ints. We can do player1 followed by p2, array of length 2*6
// * (4 (moves), every status effect, hp,
void print_state(Player* player) {
  for (int i = 0; i < 6; i++) {
    Pokemon p = player->team[i];
    if (player->active_pokemon_index == i) {
      printf("Active Pokemon: #%d: %s \n", i, get_pokemon_name(p.id));
      printf(
          "Stat Modifiers: Atk=%d, Def=%d, Spd=%d, Spec=%d, Acc=%d, "
          "Eva=%d \n",
          player->active_pokemon.stat_mods.attack,
          player->active_pokemon.stat_mods.defense,
          player->active_pokemon.stat_mods.speed,
          player->active_pokemon.stat_mods.specA,
          player->active_pokemon.stat_mods.accuracy,
          player->active_pokemon.stat_mods.evasion);
    } else {
      printf("Pokemon #%d: %s \n", i, get_pokemon_name(p.id));
    }
    printf("HP: %d \t", p.hp);
    printf("Level: %d \t", p.stats.level);
    printf("Types: %d, %d \t", p.type1, p.type2);
    printf("Available Moves: \n");
    for (int j = 0; j < 4; j++) {
      Move m = p.poke_moves[j];
      printf(
          "\t Label: %s; PP Remaining: %d; Power: %d; Accuracy: %.2f; Type: "
          "%d; Category: %d \n",
          get_move_name(m.id),
          m.pp,
          m.power,
          m.accuracy,
          m.type,
          m.category);
    }
  }
}

int get_player_choice(Player* p, int p_num, int mode) {
  // Get the player's choice of action
  int choice;
  printf("Player %d, enter your choice (0-5 to switch, 6-9 to use move): ",
         p_num);
  scanf("%d", &choice);
  while (!valid_choice(p_num, *p, choice, mode)) {
    printf(
        "Invalid choice. Please enter a valid choice (0-5 to switch, 6-9 to "
        "use move): ");
    scanf("%d", &choice);
  }
  return choice;
}

int main() {
  // seeding for testing
  srand(42);
  Sim s = {0};
  // Initialize battle and step through until a player has won
  Battle b = {0};
  Player p1 = {0};
  Player p2 = {0};
  s.battle = &b;
  s.battle->p1 = p1;
  s.battle->p2 = p2;
  sim_reset(&s);
  team_generator(&s.battle->p1);
  team_generator(&s.battle->p2);
  // Used to check what kind of moves are valid
  int mode = 0;
  while (mode < 10) {
    printf("Player 1 Info \n");
    print_state(&b.p1);
    printf("\n Player 2 Info \n");
    print_state(&b.p2);
    // Simulate a turn
    int p1_choice = get_player_choice(&b.p1, 1, mode);
    int p2_choice = get_player_choice(&b.p2, 2, mode);
    mode = step(&s);
  }
  printf("Final states: \n");
  printf("Player 1 Info \n");
  print_state(&b.p1);
  printf("\n Player 2 Info \n");
  print_state(&b.p2);
  int loser_nums = mode - 10;
  if (loser_nums == 1     ) {
    printf("Player 1 has lost! \n");
  } else if (loser_nums == 2) {
    printf("Player 2 has lost! \n");
  } else if (loser_nums == 3) {
    printf("It's a tie! Both players have lost! \n");
  }
  return 0;
}
