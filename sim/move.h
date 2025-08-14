#ifndef MOVE_H
#define MOVE_H

#include <stddef.h>  // For NULL definition

#include "generated_move_enum.h"
#include "pokemon.h"
#include "typing.h"

inline int max(int a, int b) { return (a > b) ? a : b; }

struct STR_MOVES {
  MOVE_IDS id;
  // Moves are currently being defined as functions.
  // each move obj contains a pointer to it'
  //  is a pointer to said function.
  int power;
  float accuracy;
  TYPE type;
  MOVE_CATEGORY category;  // Add category for physical or special moves
  int pp;
  int (*movePtr)(Battle*, Pokemon*, Pokemon*, int);

  int priority;

} typedef Move;


// Source: https://bulbapedia.bulbagarden.net/wiki/Damage
inline int calculate_damage(BattlePokemon* attacker, BattlePokemon* defender,
                            Move* used_move) {
  // Base power of the move
  int power = used_move->power;
  
  //These never cause damage: error state reached
  // if(used_move->category == STATUS_MOVE_CATEGORY) {
  // }

  // Attack and Defense stats
  // Only if SPECIAL attack

  Pokemon* base_attacker = attacker->pokemon;
  Pokemon* base_defender = defender->pokemon;
  int attack_stat;
  int defense_stat;

  if(used_move->category == SPECIAL_MOVE_CATEGORY) {
    attack_stat = base_attacker->stats.base_stats[STAT_SPECIAL_ATTACK] *
    
                  (attacker->stat_mods[STAT_SPECIAL_ATTACK] + 6) / 6;
    attack_stat = base_attacker->stats.base_stats[STAT_SPECIAL_ATTACK] *
                  attacker.stat_mods[STAT_SPECIAL_ATTACK];
    defense_stat = base_defender->stats.base_stats[STAT_SPECIAL_DEFENSE] *
                   defender.stat_mods[STAT_SPECIAL_DEFENSE];
  }
  if (used_move->category == PHYSICAL_MOVE_CATEGORY) {
    attack_stat = base_attacker->stats.base_stats[STAT_ATTACK] *
                  attacker.stat_mods[STAT_ATTACK];
    defense_stat = base_defender->stats.base_stats[STAT_DEFENSE] *
                   defender->stat_mods[STAT_DEFENSE];
  }
  int level = base_attacker->stats.level;
  // Type effectiveness
  float type_effectiveness =
      damage_chart[used_move->type][defender->type1] *
      damage_chart[used_move->type][defender->type2];

  // STAB (Same-Type Attack Bonus)
  float stab = (attacker->type1 == used_move->type ||
                attacker->type2 == used_move->type)
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

//Note: not handling misses here. This may prove problematic
inline void attack(Battle* b, BattlePokemon* attacker, BattlePokemon* defender,
                   Move* used_move) {
  // Calculate damage
  if (used_move->power != 0) {
    int damage = calculate_damage(attacker, defender, used_move);
    defender->pokemon->hp -= damage;
    defender->pokemon->hp = max(defender->pokemon->hp, 0);  // Ensure HP doesn't go below 0

    //Maybe a flag should be set for pre-move vs. post-move actions. I.E stall for hyper beam?
    //Or, all moves that don't have a pointer go to above, all moves that do go straight here: nicher moves can handle themselves
    if (used_move->movePtr != NULL) {
      used_move->movePtr(b, attacker, defender, damage);
    }
    // todo: implement bide counter & whatnot
  }
}

#endif