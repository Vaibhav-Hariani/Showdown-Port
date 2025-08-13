#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "generated_move_enum.h"
#include "generated_movedex.h"
#include "move_labels.h"
#include "pokedex_labels.h"
#include "pokemon.h"
#include "pokemon_entries.h"
#include "pokemon_types.h"

// Performs a battle between a bulbasaur and a squirtle
int main() {
  srand(time(NULL));
  BattlePokemon poke1, poke2;
  Gen1DV dvs = {15, 10, 10, 10};
  Gen1StatExperience evs = {0, 0, 0, 0, 0};
  // Need to double check that this doesn't have to be a memcpy
  // because of pointers
  Move BulbasaurMoves[4] = {MOVES[TACKLE_MOVE_ID],
                            MOVES[GROWL_MOVE_ID],
                            MOVES[VINE_WHIP_MOVE_ID],
                            MOVES[RAZOR_LEAF_MOVE_ID]};
  Move SquirtleMoves[4] = {MOVES[TACKLE_MOVE_ID],
                           MOVES[TAIL_WHIP_MOVE_ID],
                           MOVES[BUBBLE_MOVE_ID],
                           MOVES[WATER_GUN_MOVE_ID]};
  build_pokemon(
      &poke1, BlastoiseEntry, BlastoiseBaseStats, dvs, evs, 50, SquirtleMoves);
  build_pokemon(
      &poke2, VenusaurEntry, VenusaurBaseStats, dvs, evs, 50, BulbasaurMoves);

  // Battle loop
  int turn = 0;
  printf("%s has %d HP remaining.\n",
         POKEMON_NAMES[poke1.entry.species],
         poke1.hp);
  printf("%s has %d HP remaining.\n",
         POKEMON_NAMES[poke2.entry.species],
         poke2.hp);

  while (poke1.hp > 0 || poke2.hp > 0) {
    BattlePokemon *first = &poke1, *second = &poke2;
    printf("Turn %d:\n", turn);

    // Choose a random action for each pokemon
    int move1 = rand() % 4;
    int move2 = rand() % 4;
    while (poke1.moves[move1].move_id == NO_MOVE ||
           poke1.moves[move1].pp == 0) {
      move1 = rand() % 4;
    }
    while (poke2.moves[move2].move_id == NO_MOVE ||
           poke2.moves[move2].pp == 0) {
      move2 = rand() % 4;
    }

    // check for speed
    int first_speed = first->speed;
    int second_speed = second->speed;
    if (first->status.paralyzed) {
      first_speed /= 4;
    }
    if (second->status.paralyzed) {
      second_speed /= 4;
    }

    if (second_speed > first_speed) {
      first = &poke2;
      second = &poke1;
      int tmp = move1;
      move1 = move2;
      move2 = tmp;
    }

    // check for priority
    if (second->moves[move1].priority > first->moves[move2].priority) {
      first = &poke1;
      second = &poke2;
      int tmp = move1;
      move1 = move2;
      move2 = tmp;
    }

    // check frozen
    if (!first->status.freeze) {
      // perform first move
      apply_move(first, move1, second, move2);
      printf("%s uses %s!\n",
             POKEMON_NAMES[first->entry.species],
             MOVE_LABELS[first->moves[move1].move_id]);
      printf("%s has %d HP remaining.\n",
             POKEMON_NAMES[second->entry.species],
             second->hp);

      if (second->hp <= 0) {
        break;
      }
    }

    if (!second->status.freeze) {
      // perform second move
      apply_move(second, move2, first, move1);
      printf("%s uses %s!\n",
             POKEMON_NAMES[second->entry.species],
             MOVE_LABELS[second->moves[move2].move_id]);
      printf("%s has %d HP remaining.\n",
             POKEMON_NAMES[first->entry.species],
             first->hp);

      if (first->hp <= 0) {
        break;
      }
    }
    turn++;
  }

  printf("Battle over in %d turns!\n", turn);
  if (poke1.hp <= 0) {
    printf("%s wins!\n", POKEMON_NAMES[poke2.entry.species]);
  } else {
    printf("%s wins!\n", POKEMON_NAMES[poke1.entry.species]);
  }
}
