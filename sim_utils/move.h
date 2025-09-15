#ifndef MOVE_H
#define MOVE_H

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "battle_structs.h"
#include "move_structs.h"
#include "poke_structs.h"
#include "queue_structs.h"
#include "utils.h"
// Source: https://bulbapedia.bulbagarden.net/wiki/Damage
static inline int calculate_damage(BattlePokemon* attacker,
                                   BattlePokemon* defender,
                                   Move* used_move) {
  // Base power of the move
  int power = used_move->power;

  // Attack and Defense stats
  // Only if SPECIAL attack

  Pokemon* base_attacker = attacker->pokemon;
  Pokemon* base_defender = defender->pokemon;
  int attack_stat;
  int defense_stat;

  // Critical hits ignore the stat modifiers.
  // How this is handled should be decided separately

  // TODO: Not handling stat modifiers quite yet
  if (used_move->category == SPECIAL_MOVE_CATEGORY) {
    attack_stat = base_attacker->stats.base_stats[STAT_SPECIAL_ATTACK];
    defense_stat = base_defender->stats.base_stats[STAT_SPECIAL_DEFENSE];
  } else if (used_move->category == PHYSICAL_MOVE_CATEGORY) {
    attack_stat = base_attacker->stats.base_stats[STAT_ATTACK];
    defense_stat = base_defender->stats.base_stats[STAT_DEFENSE];
  }

  int level = base_attacker->stats.level;
  // Type effectiveness
  float type_effectiveness = damage_chart[used_move->type][defender->type1] *
                             damage_chart[used_move->type][defender->type2];

  // STAB (Same-Type Attack Bonus)
  float stab =
      (attacker->type1 == used_move->type || attacker->type2 == used_move->type)
          ? 1.5
          : 1.0;
  // Damage formula
  int damage =
      (((2 * level / 5 + 2) * power * attack_stat / defense_stat) / 50 + 2) *
      stab * type_effectiveness;
  // Damage at this point should be 0,1, or greater than 1. Only if greater
  // than one should anything happen.
  if (damage <= 1) {
    if (damage == 0) {
      DLOG("%s's attack missed!", get_pokemon_name(attacker->pokemon->id));
    }
    return damage;  // return 0 for failure
  }
  // Random factor (Exactly as specified by bulbapedia)
  float random_factor = (rand() % 38 + 217) / 255.0;
  return damage * random_factor;
}

// Todo:
// Add status checks for flinching and other statuses
//  Check for critical moves and other effects.
// Pre-move checker: applies status effects, checks recharge/flinch, and handles
// PP Returns 1 if move can proceed, 0 if blocked (status/recharge/flinch/PP)
static inline int pre_move_check(BattlePokemon* attacker, Move* used_move) {
  // Check for confusion
  int early_ret = 1;
  if (attacker->confusion_counter > 0) {
    attacker->confusion_counter--;
    DLOG("%s is confused!", get_pokemon_name(attacker->pokemon->id));
    // 50% chance to hurt itself
    if (rand() % 2 == 0) {
      // Gen 1 confusion self-damage: 40 base power typeless physical attack
      int level = attacker->pokemon->stats.level;
      int atk = attacker->pokemon->stats.base_stats[STAT_ATTACK];
      int def = attacker->pokemon->stats.base_stats[STAT_DEFENSE];
      int damage = (((2 * level / 5 + 2) * 40 * atk / def) / 50 + 2);
      float random_factor = (rand() % 38 + 217) / 255.0;
      damage = (int)(damage * random_factor);
      attacker->pokemon->hp -= damage;
      attacker->pokemon->hp = max(attacker->pokemon->hp, 0);
      DLOG("%s hurt itself in its confusion! (%d HP)",
           get_pokemon_name(attacker->pokemon->id),
           damage);
      early_ret = 0;
    }
  }
  // Check for recharge (e.g., Hyper Beam)
  if (attacker->recharge_counter > 0) {
    DLOG("%s must recharge!", get_pokemon_name(attacker->pokemon->id));
    attacker->recharge_counter--;
    // Call the function at recharge_id. Don't call the input move..
    early_ret = 10;
  }
  // Check for flinch
  if (attacker->flinch) {
    DLOG("%s flinched and couldn't move!",
         get_pokemon_name(attacker->pokemon->id));
    attacker->flinch = 0;  // Flinch is cleared after blocking move
    early_ret = 0;
  }
  // Check for sleep (Gen 1: can't act if asleep)
  if (attacker->pokemon->status.sleep > 0) {
    attacker->pokemon->status.sleep--;
    if (attacker->pokemon->status.sleep == 0) {
      DLOG("%s woke up!", get_pokemon_name(attacker->pokemon->id));
    } else {
      DLOG("%s is asleep and can't move!",
           get_pokemon_name(attacker->pokemon->id));
    }
    early_ret = 0;
  }
  if (attacker->pokemon->status.freeze) {
    DLOG("%s is frozen solid and can't move!",
         get_pokemon_name(attacker->pokemon->id));
    early_ret = 0;
  }
  // Check for PP (except Struggle)
  if (early_ret && used_move->id != STRUGGLE_MOVE_ID) {
    used_move->pp--;
  }
  // If move is valid, deduct PP (except Struggle)
  return early_ret ? 1 : 0;  // return 1 if move can proceed, 0 if blocked
}

inline int attack(Battle* b,
                  BattlePokemon* attacker,
                  BattlePokemon* defender,
                  Move* used_move) {
  // Pre-move check: status, recharge, flinch, PP
  int pre = pre_move_check(attacker, used_move);
  if (!pre) {
    return 0;
  }
  // Recharging
  if (pre == 10) {
    Move m = attacker->recharge_src;
    m.movePtr(b, attacker, defender);
  }
  if (used_move->power != 0) {
    DLOG("%s used %s!",
         get_pokemon_name(attacker->pokemon->id),
         get_move_name(used_move->id));

    // Calculate damage
    int damage = calculate_damage(attacker, defender, used_move);
    // Not this simple with substitutes and whatnot: might need an apply_damage
    // function.
    defender->pokemon->hp -= damage;
    defender->pokemon->hp =
        max(defender->pokemon->hp, 0);  // Ensure HP doesn't go below 0
  }
  // Handle move-specific logic
  if (used_move->movePtr != NULL) {
    used_move->movePtr(b, attacker, defender);
  }
  return 1;
}

int valid_move(Player* user, int move_index) {
  if(user->active_pokemon_index < 0) {
    //This is a bugged state: Should never arrive here.
    // The pokemon should have been forced to switch out by now
    return 0;
  }
  Move m = user->active_pokemon.pokemon->poke_moves[move_index];
  if (m.pp <= 0 && m.id != STRUGGLE_MOVE_ID) {
    DLOG("Move %s has no PP left!", get_move_name(m.id));
    return 0;
  }
  return 1;
}

// Adds a move to the battleQueue. Returns 0 if move is invalid (PP too low), 1
// if added.
int add_move_to_queue(Battle* battle,
                      Player* user,
                      Player* target,
                      int move_index) {
  // Assumes input is screened beforehand.
  BattlePokemon* battle_poke = &user->active_pokemon;
  Move* move = (battle_poke->pokemon->poke_moves) + move_index;
  // Add to queue by modifying the action at q_size
  if (battle->action_queue.q_size < 15) {
    Action* action_ptr =
        &battle->action_queue.queue[battle->action_queue.q_size];
    memset(action_ptr, 0, sizeof(Action));  // Clear any previous data
    action_ptr->action_type = move_action;
    action_ptr->action_d.m = *move;
    action_ptr->User = user;
    action_ptr->Target = target;
    action_ptr->order = 200;  // Use 200 as the order for moves
    action_ptr->priority = move->priority;
    // TODO: Speed should be impacted by the pokemon's stat modifiers
    action_ptr->speed = battle_poke->pokemon->stats.base_stats[STAT_SPEED];
    action_ptr->origLoc = user->active_pokemon_index;
    DLOG("Added move %s to queue for %s.",
         get_move_name(move->id),
         get_pokemon_name(battle_poke->pokemon->id));
    return 1;
  } else {
    // Need to crash
    DLOG("Battle queue is full!\n");
    exit(1);
    return 0;  // Changed from -2 to 0 for standardized return values
  }
}

#endif
