#ifndef MOVE_H
#define MOVE_H

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../data_sim/generated_movedex.h"
#include "../data_sim/stat_modifiers.h"
#include "battle_structs.h"
#include "move_structs.h"
#include "poke_structs.h"
#include "queue_structs.h"
#include "utils.h"
// for memset
#include "string.h"

int calculate_damage(BattlePokemon* attacker,
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
  // TODO: Focus energy also has a special critical hit effect.
  // How this is handled should be decided separately

  // Get stat modifiers as fixed-point values (256 = 1.0x)
  if (used_move->category == SPECIAL_MOVE_CATEGORY) {
    attack_stat = base_attacker->stats.base_stats[STAT_SPECIAL_ATTACK];
    if (attacker->pokemon->status.burn) {
      attack_stat /= 2;  // Burn halves physical attack
    }
    // Apply stat modifier using fixed-point arithmetic
    attack_stat = (attack_stat * get_stat_modifier(attacker->stat_mods.specA)) >> 8;
    // Specials use stat_special_defence: this should always be the same as
    // their stat_special
    defense_stat = base_defender->stats.base_stats[STAT_SPECIAL_DEFENSE];
    defense_stat = (defense_stat * get_stat_modifier(defender->stat_mods.specD)) >> 8;
    if (defender->light_screen) {
      defense_stat *= 2;  // Light Screen doubles special defense
      if (defense_stat > 1024) {
        defense_stat -= defense_stat % 1024;
      }
    }
  } else if (used_move->category == PHYSICAL_MOVE_CATEGORY) {
    attack_stat = base_attacker->stats.base_stats[STAT_ATTACK];
    attack_stat = (attack_stat * get_stat_modifier(attacker->stat_mods.attack)) >> 8;
    defense_stat = base_defender->stats.base_stats[STAT_DEFENSE];
    defense_stat = (defense_stat * get_stat_modifier(defender->stat_mods.defense)) >> 8;
    if (defender->reflect) {
      defense_stat *= 2;  // Reflect doubles physical defense
    }
    if (defense_stat > 1024) {
      defense_stat -= defense_stat % 1024;
    }
  }

  int level = base_attacker->stats.level;
  // Type effectiveness using fixed-point arithmetic (256 = 1.0x)
  // Multiply two damage chart values together, then divide by 256
  uint32_t type_effectiveness = 
      ((uint32_t)damage_chart[used_move->type][defender->type1] * 
       (uint32_t)damage_chart[used_move->type][defender->type2]) >> 8;

  // STAB (Same-Type Attack Bonus): 1.5x = 384/256, 1.0x = 256/256
  uint16_t stab =
      (attacker->type1 == used_move->type || attacker->type2 == used_move->type)
          ? 384  // 1.5x in fixed-point
          : 256; // 1.0x in fixed-point
  
  // Damage formula with fixed-point arithmetic
  // Base damage calculation (integer portion)
  int base_damage = ((2 * level / 5 + 2) * power * attack_stat / defense_stat) / 50 + 2;
  
  // Apply type effectiveness and STAB using fixed-point
  // (base_damage * stab * type_effectiveness) / (256 * 256)
  int damage = (base_damage * stab * type_effectiveness) >> 16;
  
  // Damage at this point should be 0,1, or greater than 1. Only if greater
  // than one should anything happen.
  if (damage <= 1) {
    if (damage == 0) {
      DLOG("%s's attack missed!", get_pokemon_name(attacker->pokemon->id));
    }
    return damage;  // return 0 for failure
  }
  // Random factor (Exactly as specified by bulbapedia)
  // Use integer arithmetic: multiply then divide to avoid float
  int random_factor = rand() % 38 + 217;  // 217-254 range
  return (damage * random_factor) / 255;
}

// Add status checks for flinching and other statuses
//  Check for critical moves and other effects.
// Pre-move checker: applies status effects, checks recharge/flinch, and handles
// PP Returns 1 if move can proceed, 0 if blocked (status/recharge/flinch/PP)
static inline int pre_move_check(BattlePokemon* attacker, Move* used_move) {
  if (attacker->immobilized) {
    DLOG("%s is trapped and can't move!",
         get_pokemon_name(attacker->pokemon->id));
    return 0;
  }
  // Check if multi-turn move is disabled (for ongoing moves like Bind/Wrap)
  if (attacker->disabled_count > 0 && attacker->multi_move_src != NULL &&
      attacker->multi_move_src->id == attacker->disabled_move_id) {
    DLOG("%s's %s is disabled!",
         get_pokemon_name(attacker->pokemon->id),
         get_move_name(used_move->id));
    // Clear the multi-turn move
    attacker->multi_move_len = 0;
    attacker->multi_move_src = NULL;
    return 0;
  }
  // Check for confusion
  int regular_return = 1;
  if (attacker->confusion_counter > 0) {
    attacker->confusion_counter--;
    DLOG("%s is confused!", get_pokemon_name(attacker->pokemon->id));
    // 50% chance to hurt itself
    if (rand() % 2 == 0) {
      // Gen 1 confusion self-damage: 40 base power typeless physical attack
      int level = attacker->pokemon->stats.level;
      int atk = attacker->pokemon->stats.base_stats[STAT_ATTACK];
      int def = attacker->pokemon->stats.base_stats[STAT_DEFENSE];
      if (attacker->reflect) {
        def *= 2;  // Reflect doubles physical defense
      }
      int damage = (((2 * level / 5 + 2) * 40 * atk / def) / 50 + 2);
      float random_factor = (rand() % 38 + 217) / 255.0;
      damage = (int)(damage * random_factor);
      attacker->pokemon->hp -= damage;
      attacker->pokemon->hp = max(attacker->pokemon->hp, 0);
      DLOG("%s hurt itself in its confusion! (%d HP)",
           get_pokemon_name(attacker->pokemon->id),
           damage);
      regular_return = 0;
    }
  }
  // Check for recharge (e.g., Hyper Beam)
  if (attacker->recharge_counter > 0) {
    return 10;
  }
  // Check for flinch
  if (attacker->flinch) {
    DLOG("%s flinched and couldn't move!",
         get_pokemon_name(attacker->pokemon->id));
    attacker->flinch = 0;  // Flinch is cleared after blocking move
    regular_return = 0;
  }
  // Check for sleep (Gen 1: can't act if asleep)
  if (attacker->sleep_ctr > 0) {
    DLOG("%s is asleep and can't move!",
         get_pokemon_name(attacker->pokemon->id));
    regular_return = 0;
  }
  if (attacker->pokemon->status.freeze) {
    DLOG("%s is frozen solid and can't move!",
         get_pokemon_name(attacker->pokemon->id));
    regular_return = 0;
  }
  // Check for PP (except Struggle and if locked into Rage)
  // printf("VALUES %d %p %p\n", regular_return, attacker, used_move);
  // printf("RAGE %p\n", attacker->rage);
  // printf("STRUGGLE %d\n", used_move->id == STRUGGLE_MOVE_ID);
  if (regular_return && attacker->rage == NULL &&
      used_move->id != STRUGGLE_MOVE_ID) {
    // Do not block here for PP exhaustion; attack() will handle replacing
    // no-PP moves with Struggle so that we can still proceed when all
    // moves are out of PP. If move has PP, consume one.
    if (used_move->pp > 0) {
      used_move->pp--;
    } else {
      DLOG("%s has no PP left for %s!",
           get_pokemon_name(attacker->pokemon->id),
           get_move_name(used_move->id));
      regular_return = 0;
    }
  }
  return regular_return ? 1 : 0;  // return 1 if move can proceed, 0 if blocked
}

inline int attack(Battle* b,
                  BattlePokemon* attacker,
                  BattlePokemon* defender,
                  Move* used_move) {
  if (used_move == NULL) {
    DLOG("Attempt to use invalid move");
  }
  if (attacker->rage != NULL && used_move->id != RAGE_MOVE_ID) {
    DLOG("%s is locked into Rage!", get_pokemon_name(attacker->pokemon->id));
    used_move = attacker->rage;
  }

  // If the selected move has no PP (and is not Struggle), replace it with
  // a temporary Struggle move so the attack proceeds as Struggle.
  if (used_move->pp <= 0 && used_move->id != STRUGGLE_MOVE_ID) {
    used_move = &attacker->pokemon->struggle;
    b->lastMove = used_move;
    used_move->revealed = 1;
    attacker->last_used = used_move;
  }

  int pre = pre_move_check(attacker, used_move);
  if (!pre) {
    return 0;
  }
  int is_recharge_turn = (pre == 10);
  if (pre == 10) {
    used_move = &attacker->recharge_src;
    if (used_move->id == NO_MOVE) {
      attacker->recharge_counter = 0;
      attacker->recharge_len = 0;
      if (attacker->no_switch == SWITCH_STOP_SOLAR_BEAM) {
        attacker->no_switch = SWITCH_STOP_NONE;
      }
      return 1;
    }
  }

  b->lastMove = used_move;
  used_move->revealed = 1;
  attacker->last_used = used_move;

  int is_solar_beam_charge =
      (!is_recharge_turn && used_move->id == SOLAR_BEAM_MOVE_ID &&
       attacker->recharge_counter == 0 && attacker->recharge_len == 0);

  if (is_solar_beam_charge) {
    if (used_move->movePtr != NULL) {
      used_move->movePtr(b, attacker, defender);
    }
    return 1;
  }

  // Accuracy is now stored as uint8_t (0-255), so no need to multiply
  // Use fixed-point stat modifiers (256 = 1.0x)
  int accuracy = ((int)used_move->accuracy * 
                  get_stat_modifier(attacker->stat_mods.accuracy) *
                  get_evasion_modifier(defender->stat_mods.evasion)) >> 16;
  int accuracy_random = (rand() % 256);
  if (accuracy_random >= accuracy) {
    DLOG("%s's attack %s missed!",
         get_pokemon_name(attacker->pokemon->id),
         get_move_name(used_move->id));

    if (used_move->id == HIGH_JUMP_KICK_MOVE_ID) {
      attacker->pokemon->hp = max(attacker->pokemon->hp - 1, 0);
    }
    if (used_move->id == RAGE_MOVE_ID) {
      attacker->no_switch = SWITCH_STOP_RAGE;
      attacker->rage = used_move;
    }
    if (used_move->id == SOLAR_BEAM_MOVE_ID) {
      attacker->recharge_counter = 0;
      attacker->recharge_len = 0;
      attacker->recharge_src = (Move){0};
      if (attacker->no_switch == SWITCH_STOP_SOLAR_BEAM) {
        attacker->no_switch = SWITCH_STOP_NONE;
      }
    }
    return 0;
  }

  int damage = 0;
  int skip_damage = (used_move->id == SOLAR_BEAM_MOVE_ID);
  if (!skip_damage && used_move->power != 0 &&
      attacker->recharge_counter == attacker->recharge_len) {
    DLOG("%s used %s!",
         get_pokemon_name(attacker->pokemon->id),
         get_move_name(used_move->id));

    damage = calculate_damage(attacker, defender, used_move);

    if (attacker->recharge_len == 0 ||
        attacker->recharge_counter != attacker->recharge_len) {
      if (defender->substitute_hp > 0) {
        if (damage >= defender->substitute_hp) {
          defender->substitute_hp = 0;
          damage = 0;
        } else {
          defender->substitute_hp -= damage;
          damage = 0;
        }
      } else {
        defender->pokemon->hp -= damage;
        defender->pokemon->hp = max(defender->pokemon->hp, 0);
      }
      defender->dmg_counter += damage;
    }
    b->lastDamage = damage;
  }

  if (used_move->movePtr != NULL) {
    used_move->movePtr(b, attacker, defender);
  }

  if (used_move->id == HYPER_BEAM_MOVE_ID) {
    used_move->power = 0;
  }

  if (defender->pokemon->status.freeze && used_move->type == FIRE &&
      used_move->id != FIRE_SPIN_MOVE_ID) {
    attacker->pokemon->status.freeze = 0;
    DLOG("%s thawed out!", get_pokemon_name(attacker->pokemon->id));
  }
  return 1;
}

int valid_move(Player* user, int move_index) {
  if (user->active_pokemon_index < 0) {
    return 0;
  }
  Move* move = &user->active_pokemon.moves[move_index];
  if (move == NULL || move->id == NO_MOVE) {
    DLOG("Attempt to use invalid move slot %d", move_index);
    return 0;
  }
  if (user->active_pokemon.disabled_count > 0 &&
      move->id == user->active_pokemon.disabled_move_id) {
    DLOG("Attempt to use disabled move %s", get_move_name(move->id));
    return 0;
  }
  // If the move has no PP, normally it's invalid – but if the player has
  // no other moves with PP remaining, allow the move so the engine can
  // resolve it as Struggle.
  if (move->pp <= 0 && move->id != STRUGGLE_MOVE_ID) {
    DLOG("Move %s has no PP left!", get_move_name(move->id));
    for (int i = 0; i < 4; i++) {
      Move* m = &user->active_pokemon.moves[i];
      if (m && m->id != NO_MOVE && m->pp > 0) {
        return 0;
      }
    }
    DLOG("No valid moves... %s used Struggle!",
         get_pokemon_name(user->active_pokemon.pokemon->id));
    return 1;
  }

  return 1;
}

// Adds a move to the battleQueue. Returns 0 if move is invalid (PP too low),
// 1 if added.
int add_move_to_queue(Battle* battle,
                      Player* user,
                      Player* target,
                      int move_index) {
  // Assumes input is screened beforehand.
  BattlePokemon* battle_poke = &user->active_pokemon;
  Move* move = (battle_poke->pokemon->poke_moves) + move_index;
  // In the case of transform, mimic.
  if (battle_poke->moves[move_index].id != 0) {
    move = battle_poke->moves + move_index;
  }
  // Add to queue by modifying the action at q_size
  if (battle->action_queue.q_size < ACTION_QUEUE_MAX) {
    Action* action_ptr =
        &battle->action_queue.queue[battle->action_queue.q_size];
    memset(action_ptr, 0, sizeof(Action));  // Clear any previous data
    action_ptr->action_type = move_action;
    action_ptr->action_d.m = move;
    action_ptr->User = user;
    action_ptr->Target = target;
    action_ptr->order = 200;  // Use 200 as the order for moves
    action_ptr->priority = move->priority;

    int base_speed = battle_poke->pokemon->stats.base_stats[STAT_SPEED];
    action_ptr->speed =
        base_speed * get_stat_modifier(battle_poke->stat_mods.speed);
    if (battle_poke->pokemon->status.paralyzed) {
      action_ptr->speed /= 4;
    }
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
