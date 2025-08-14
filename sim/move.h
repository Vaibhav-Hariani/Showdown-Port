#ifndef MOVE_H
#define MOVE_H

#include <stddef.h>  // For NULL definition

#include "move_enum.h"
#include "pokemon.h"
#include "typing.h"

inline int max(int a, int b) { return (a > b) ? a : b; }

struct STR_MOVES {
  move_ids id;
  // Moves are currently being defined as functions.
  // each move obj contains a pointer to it'
  //  is a pointer to said function.
  int (*movePtr)(battle*, pokemon*, pokemon*, int);
  int power;
  int accuracy;
  TYPE move_type;
  int pp;
  move_category category;  // Add category for physical or special moves
} typedef move;


// Source: https://bulbapedia.bulbagarden.net/wiki/Damage
inline int calculate_damage(pokemon* attacker, pokemon* defender,
                            move* used_move) {
  // Base power of the move
  int power = used_move->power;

  // Attack and Defense stats
  // Only if SPECIAL attack
  int attack_stat = attacker->stats.base_stats[STAT_SPECIAL_ATTACK] *
                    attacker->mods.stat_mods[STAT_SPECIAL_ATTACK];
  int defense_stat = defender->stats.base_stats[STAT_SPECIAL_DEFENSE] *
                     defender->mods.stat_mods[STAT_SPECIAL_DEFENSE];

  if (used_move->category == PHYSICAL_MOVE) {
    attack_stat = attacker->stats.base_stats[STAT_ATTACK] *
                  attacker->mods.stat_mods[STAT_ATTACK];
    defense_stat = defender->stats.base_stats[STAT_DEFENSE] *
                   defender->mods.stat_mods[STAT_DEFENSE];
  }

  int level = attacker->level;
  // Type effectiveness
  float type_effectiveness =
      damage_chart[used_move->move_type][defender->type1] *
      damage_chart[used_move->move_type][defender->type2];

  // STAB (Same-Type Attack Bonus)
  float stab = (attacker->type1 == used_move->move_type ||
                attacker->type2 == used_move->move_type)
                   ? 1.5
                   : 1.0;
  // Damage formula
  int damage =
      (((2 * level / 5 + 2) * power * attack_stat / defense_stat) / 50 + 2) *
      stab * type_effectiveness;
  // Damage at this point should be 0,1, or greater than 1. Only if greater than one should anything happen.
  if (damage <= 1) {
    //Log a miss if this doesn't happen
    return damage;
  }
  // Random factor (Exactly as specified by bulbapedia)
  float random_factor = (rand() % 38 + 217) / 255.0;
  return damage * random_factor;
}

inline void attack(battle* b, pokemon* attacker, pokemon* defender,
                   move* used_move) {
  // Calculate damage
  if (used_move->power != 0) {
    int damage = calculate_damage(attacker, defender, used_move);
    defender->hp -= damage;
    defender->hp = max(defender->hp, 0);  // Ensure HP doesn't go below 0

    //Maybe a flag should be set for pre-move vs. post-move actions. I.E stalls for recharge moves
    if (used_move->movePtr != NULL) {
      used_move->movePtr(b, attacker, defender, damage);
    }
    // todo: implement bide counter & whatnot
  }
}

#endif