#include "sim.h"

//TODO: Fix this entire system. Mega-borked


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
  s.observations = (int16_t*)calloc(108, sizeof(int16_t));
  Battle bat = {0};
  float reward = 0.0f;
  s.battle = &bat;
  s.rewards = &reward;
  c_reset(&s);
  team_generator(&s.battle->p1);
  team_generator(&s.battle->p2);
  // Used to check what kind of moves are valid
  Battle* b = s.battle;

  while (s.rewards[0] * s.rewards[0] != 1.0f) {
    printf("Player 1 Info \n");
    print_state(&b->p1);
    printf("\n Player 2 Info \n");
    print_state(&b->p2);
    // Simulate a turn
    printf("Reward: %.2f \n", s.rewards[0]);
    int p1_choice = get_player_choice(&b->p1, 1, b->mode);
    // int p2_choice = get_player_choice(&b->p2, 2, mode);
    battle_step(&s, p1_choice);
    if (b->mode == 0) {
      end_step(b);
    }
    s.battle->action_queue.q_size = 0;
  }
  printf("Player 1 Info \n");
  print_state(&b->p1);
  printf("\n Player 2 Info \n");
  print_state(&b->p2);
  return 0;
}
