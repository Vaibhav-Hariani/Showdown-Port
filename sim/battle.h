// Effectively, the runner.
//  Sourced from battle.ts

#ifndef BATTLE_H
#define BATTLE_H
#include "pokemon.h"
#include "move.h"
// Their battle object contains a loooot of metadata.
// I'm combining battleOptions and battle into one "big" system

// From research
//  Can see other pokemon HP percentage as well as stat percentage
//  Status effects are revealed
//  Cannot see entire team: only pokemon they played already. This can be
//  simulated with missingNo for unknowns?
// For gen1:
// Desync Clause Mod: Desyncs changed to move failure.
// Sleep Clause Mod: Limit one foe put to sleep
// Freeze Clause Mod: Limit one foe frozen
// Species Clause: Limit one of each Pokémon
// OHKO Clause: OHKO moves are banned
// Evasion Moves Clause: Evasion moves are banned
// Endless Battle Clause: Forcing endless battles is banned
// HP Percentage Mod: HP is shown in percentages

struct STR_PLAYER {
  pokemon team[6];
  char active_pokemon;
} typedef player;


struct STR_BATTLE {
  int seed;
  player p1;
  player p2;
  char cur_player;
  // No field, as weather effects weren't a thing in gen1
  battlequeue action_queue;
  // Major modification happening here by moving Move actions outside of a
  // BattleActions class This should streamline things (hopefully)?
  int turn_num;

  // Metadata for stuff like counter
  move* lastMove;
  int lastDamage;
  // Effects, events, and active pokemon go here
  // Unsure how this should be structured:
  // It seems the architecture relies on these being set, and then handles
  // poison/toxic and other callback loops Leaving them out for now, but may
  // need them in the near future
} typedef battle;

// Move calculate_damage to battle.h
int calculate_damage(pokemon *attacker, pokemon *defender, move *used_move) {
    // Base power of the move
    int power = used_move->power;

    // Attack and Defense stats
    int attack_stat, defense_stat;
    if (used_move->category == PHYSICAL) {
        attack_stat = attacker->stat.stats[STAT_ATTACK];
        defense_stat = defender->stat.stats[STAT_DEFENSE];
    } else { // SPECIAL
        attack_stat = attacker->stat.stats[STAT_SPECIAL_ATTACK];
        defense_stat = defender->stat.stats[STAT_SPECIAL_DEFENSE];
    }

    // Level of the attacker
    int level = 50; // Default level for now; can be parameterized later

    // Type effectiveness
    float type_effectiveness = damage_chart[used_move->move_type][defender->type1];
    if (defender->type2 != NONETYPE) {
        type_effectiveness *= damage_chart[used_move->move_type][defender->type2];
    }

    // Random factor (between 85% and 100%)
    float random_factor = (rand() % 16 + 85) / 100.0;

    // STAB (Same-Type Attack Bonus)
    float stab = (attacker->type1 == used_move->move_type || attacker->type2 == used_move->move_type) ? 1.5 : 1.0;

    // Damage formula
    int damage = (((2 * level / 5 + 2) * power * attack_stat / defense_stat) / 50 + 2) * stab * type_effectiveness * random_factor;

    // Ensure damage is at least 1
    if (damage < 1) {
        damage = 1;
    }

    return damage;
}
#endif