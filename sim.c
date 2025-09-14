#include "sim.h"

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
