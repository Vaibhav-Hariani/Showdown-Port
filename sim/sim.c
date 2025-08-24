#include "battle.h"
#include "battle_queue.h"
#include "move.h"
#include "pokedex.h"
#include "pokemon.h"
#include "stdio.h"
Battle b = {0};

// ToDo: implement init_battle(), evaluate the queue
// Then, add checks for status effects, misses, etc. etc. Follow OU rules, and
// implement switching mechanics.

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

inline int valid_choice(int player_num,
                        Player p,
                        unsigned int input,
                        int mode) {
  // The players input doesn't even matter
  if (mode != player_num && mode != 3 && mode != 0) {
    return 1;
  }

  if (input <= 6 && valid_switch(p, input)) {
    return 1;
  }

  if (mode == 0 && input <= 10 && valid_move(p, input - 7)) {
    return 1;
  }

  return 0;
}

// guaranteed to be correct from valid_choice
void action(Battle* b, Player* user, Player* target, int input, int type) {
  Action* cur = (b->action_queue.queue) + b->action_queue.q_size;
  b->action_queue.q_size++;
  if (input >= 7) {
    input -= 7;
    add_move_to_queue(b, user, target, input);
  } else {
    add_switch(b, user, input, type);
  }
  return;
}

// Takes inputs move from both players.
// Resolves queue until choice is required from one (or both) players
// Culls choices that aren't valid
int step(Battle* b, int p1_choice, int p2_choice, int mode) {
  if (mode == 0) {
    if (!(valid_choice(1, b->p1, p1_choice, mode) &&
          valid_choice(2, b->p2, p2_choice, mode))) {
      return -1;
    }
    action(b, &b->p1, &b->p2, p1_choice, REGULAR);
    action(b, &b->p2, &b->p1, p2_choice, REGULAR);
  } else {
    // player 1 has lost a pokemon
    if ((mode == 1 || mode == 3)) {
      if (!valid_choice(1, b->p1, p1_choice, mode)) {
        return -1;
      }
      action(b, &b->p1, &b->p2, p1_choice, FAINTED);
    }
    if ((mode == 2 || mode == 3)) {
      if (!valid_choice(2, b->p2, p2_choice, mode)) {
        return -1;
      }
      action(b, &b->p2, &b->p1, p2_choice, FAINTED);
    }
  }
  // Sort & evaluate the battlequeue on a move by move basis
  return eval_queue(b);
  // If this is greater than 0, that means a player has lost a pokemon.
}

// return 0 if nobody has won, or player_num(s) if a player has LOST.
int losers(Battle* b) {
  int losers = 0;
  for (int i = 1; i <= 2; i++) {
    int living = 0;
    Player* p = get_player(b, i);
    for (int j = 0; j < 6; j++) {
      if (p->team[j].hp > 0) {
        living = 1;
        break;
      }
    }
    if (living <= 0) {
      losers += i;
    }
  }
  return losers;
}

int get_player_choice(Player* p, int p_num, int mode){
  // Get the player's choice of action
  int choice;
  printf("Player %d, enter your choice (1-6 to switch, 7-10 to use move): ", p_num);
  scanf("%d", &choice);
  while(!valid_choice(p_num, *p, choice, mode)){
    printf("Invalid choice. Please enter a valid choice (1-6 to switch, 7-10 to use move): ");
    scanf("%d", &choice);
  }
  return choice;
}

int main() {
  // Initialize battle and step through until a player has won
  Battle b = {0};
  Player p1 = {0};
  Player p2 = {0};
  b.p1 = p1;
  b.p2 = p2;
  //Used to check what kind of moves are valid
  int mode = 0;
  int loser_nums = 0;
  while(!loser_nums) {
    printf("Player 1 Info \n");
    print_state(&b.p1);
    printf("\n Player 2 Info \n");
    print_state(&b.p2);
    // Simulate a turn
    int p1_choice = get_player_choice(&b.p1, 1,  mode);
    int p2_choice = get_player_choice(&b.p2, 2, mode);
    mode = step(&b, p1_choice, p2_choice, mode);
    loser_nums = losers(&b);
  }
  printf("Final states: \n");
  printf("Player 1 Info \n");
  print_state(&b.p1);
  printf("\n Player 2 Info \n");
  print_state(&b.p2);

  if(loser_nums == 1){
    printf("Player 1 has lost! \n");
  } else if(loser_nums == 2){
    printf("Player 2 has lost! \n");
  } else if(loser_nums == 3){
    printf("It's a tie! Both players have lost! \n");
  }
  return 0;
}
