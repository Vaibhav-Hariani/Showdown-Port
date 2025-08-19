#ifndef MOVE_H
#define MOVE_H

#include <stddef.h>  // For NULL definition
#include <stdio.h>

#include "battle.h"
#include "generated_move_enum.h"
#include "move_labels.h"
#include "pokemon.h"
#include "typing.h"
#include "log.h"
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

  // Attack and Defense stats
  // Only if SPECIAL attack

  Pokemon* base_attacker = attacker->pokemon;
  Pokemon* base_defender = defender->pokemon;
  int attack_stat;
  int defense_stat;

  //Critical hits ignore the stat modifiers. 
  //How this is handled should be decided separately

  //TODO: Not handling stat modifiers quite yet
  if (used_move->category == SPECIAL_MOVE_CATEGORY) {
    attack_stat = base_attacker->stats.base_stats[STAT_SPECIAL_ATTACK];
    defense_stat = base_defender->stats.base_stats[STAT_SPECIAL_DEFENSE];
    if (used_move->category == PHYSICAL_MOVE_CATEGORY) {
      attack_stat = base_attacker->stats.base_stats[STAT_ATTACK];
      defense_stat = base_defender->stats.base_stats[STAT_DEFENSE];
    }
    int level = base_attacker->stats.level;
    // Type effectiveness
    float type_effectiveness = damage_chart[used_move->type][defender->type1] *
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
    // Damage at this point should be 0,1, or greater than 1. Only if greater
    // than one should anything happen.
    if (damage <= 1) {
      if(damage == 0) {
        DLOG("%s's attack missed!", get_pokemon_name(attacker->pokemon));
      }
      return damage;
    }
    // Random factor (Exactly as specified by bulbapedia)
    float random_factor = (rand() % 38 + 217) / 255.0;
    return damage * random_factor;
  }
}

//Todo: 
//Add status checks for flinching and other statuses
// Check for critical moves and other effects.  
inline void attack(Battle* b, BattlePokemon* attacker, BattlePokemon* defender,
                   Move* used_move) {
  if (used_move->power != 0) {
    DLOG("%s used %s!", get_pokemon_name(attacker->pokemon), MOVE_LABELS[used_move->id]);

    // Calculate damage
    int damage = calculate_damage(attacker, defender, used_move);
    //Not this simple with substitutes and whatnot: might need an apply_damage function.
    defender->pokemon->hp -= damage;
    defender->pokemon->hp =
        max(defender->pokemon->hp, 0);  // Ensure HP doesn't go below 0
  }

  // Handle move-specific logic
  if (used_move->movePtr != NULL) {
    used_move->movePtr(b, attacker, defender, used_move);
  }
  used_move->pp--;
}

// Adds a move to the battleQueue. Returns 0 if move is invalid (PP too low), 1 if added.
inline int add_move_to_queue(Battle* battle, Player* user, Player* target, BattlePokemon* battle_poke, int move_index) {
  // Check move index bounds
  if (move_index < 0 || move_index >= 4) return 0;
  Move* move = &battle_poke->pokemon->poke_moves[move_index];
    
  if (move->pp <= 0 && move->id != STRUGGLE_MOVE_ID) {
    DLOG("Move %s has no PP left!", MOVE_LABELS[move->id]);
    return 0;
  }
  // Add to queue by modifying the action at q_size
  if (battle->action_queue.q_size < 15) {
    Action* action_ptr = &battle->action_queue.queue[battle->action_queue.q_size];
    memset(action_ptr, 0, sizeof(Action)); // Clear any previous data
    action_ptr->action_type = move_action;
    action_ptr->action_d.m = *move;
    action_ptr->User = user;
    action_ptr->Target = target;
    action_ptr->order = 200; // Use 200 as the order for moves
    action_ptr->priority = move->priority;
    // TODO: Speed should be impacted by the pokemon's stat modifiers
    action_ptr->speed = battle_poke->pokemon->stats.base_stats[STAT_SPEED];
    action_ptr->origLoc = user->active_pokemon_index;
    battle->action_queue.q_size++;
    DLOG("Added move %s to queue for %s.", MOVE_LABELS[move->id], get_pokemon_name(battle_poke->pokemon->id));
    return 1;
  } else {
    DLOG("Battle queue is full!");
    return 0;
  }
  // Add to queue by modifying the action at q_size
  if (battle->action_queue.q_size < 15) {
    Action* action_ptr = &battle->action_queue.queue[battle->action_queue.q_size];
    memset(action_ptr, 0, sizeof(Action)); // Clear any previous data
    action_ptr->action_type = move_action;
    action_ptr->action_d.m = *move;
    action_ptr->User = user;
    action_ptr->Target = target;
    action_ptr->order = 200; // Use 200 as the order for moves
    action_ptr->priority = move->priority;
    // TODO: Speed should be impacted by the pokemon's stat modifiers
    action_ptr->speed = battle_poke->pokemon->stats.base_stats[STAT_SPEED];
    action_ptr->origLoc = user->active_pokemon_index;
    battle->action_queue.q_size++;
    DLOG("Added move %s to queue for %s.", MOVE_LABELS[move->id], get_pokemon_name(battle_poke->pokemon->id));
    return 1;
  } else {
    DLOG("Battle queue is full!");
    return 0;
  }
}

#endif