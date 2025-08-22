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

void print_state(Player player) {
  for (int i = 0; i < 6; i++) {
    Pokemon p = player.team[i];
    if (player.active_pokemon_index == i) {
      DLOG("Active Pokemon: #%d: %s \n", i, get_pokemon_name(p.id));
      DLOG(
          "Stat Modifiers: Atk=%d, Def=%d, Spd=%d, Spec=%d, Acc=%d, "
          "Eva=%d \n",
          player.active_pokemon.stat_mods.attack,
          player.active_pokemon.stat_mods.defense,
          player.active_pokemon.stat_mods.speed,
          player.active_pokemon.stat_mods.specA,
          player.active_pokemon.stat_mods.accuracy,
          player.active_pokemon.stat_mods.speed);
    } else {
      DLOG("Pokemon #%d: %s \n", i, get_pokemon_name(p.id));
    }
    DLOG("HP: %d \t", p.hp);
    DLOG("Level: %d \t", p.stats.level);
    DLOG("Types: %d, %d \t", p.type1, p.type2);
    DLOG("Available Moves: \n");
    for (int j = 0; j < 4; j++) {
      Move m = p.poke_moves[j];
      DLOG(
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

inline int valid_choice(int player_num, Player p, unsigned int input, int mode) {
  if (input <= 6 && valid_switch(p, input)) {
    return 1;
  }
  else if(mode == 0 && input <= 10 && valid_move(p,input-7)) {
    return 1;
  }
  return 0;
}

//guaranteed to be correct from valid_choice
int action(Battle* b, Player* user, Player* target, int input, int type) {
  Action* cur = (b->action_queue.queue) + b->action_queue.q_size;
  b->action_queue.q_size++;
  if (input >= 7) {
    input -= 7;
    add_move_to_queue(b, user, target, input);
  } else {
    add_switch(b, user, input, type);
  }
  //Something has failed: Inputs are invalid 
  return 0;
}

// Takes inputs move from both players.
// Resolves queue until choice is required from one (or both) players
// Culls choices that aren't valid
int step(Battle* b, int p1_choice, int p2_choice, int mode) {
  if(!valid_choice(1, b->p1, p1_choice, mode) || !valid_choice(2, b->p2, p2_choice, mode)) {
    return -1;
  }
  if(mode == 0){
    action(b, &b->p1, &b->p2, p1_choice, REGULAR);
    action(b, &b->p2, &b->p1, p2_choice, REGULAR);
  } else {
  //player 1 has lost a pokemon
    if(mode == 1 || mode == 3) {
      action(b, &b->p1, &b->p2, p1_choice, FAINTED);
    } if (mode == 2 || mode == 3) {
      action(b, &b->p2, &b->p1, p2_choice, FAINTED);
    }
  }
  // Sort & evaluate the battlequeue on a move by move basis
  return eval_queue(b);
  //If this is greater than 0, that means a player has lost a pokemon.
}

int main() {
  // Initialize battle
  Battle b = {0};
  Player p1 = {0};
  Player p2 = {0};
  b.p1 = p1;
  b.p2 = p2;
  return 0;
}
