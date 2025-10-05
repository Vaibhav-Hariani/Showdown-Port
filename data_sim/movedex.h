#ifndef MOVE_DEX_H
#define MOVE_DEX_H

#include <stdbool.h>
#include <stdlib.h>

#include "../sim_utils/battle.h"
#include "../sim_utils/log.h"
#include "../sim_utils/move.h"
#include "generated_move_enum.h"
#include "typing.h"

// Move dex for Gen 1
// Moves are defined as functions that apply specific effects after damage
// calculation Each move has a name, type, power, accuracy, and PP defined in
// movedex.csv Reference: https://bulbapedia.bulbagarden.net/wiki/Damage

// ============================================================================
// HELPER FUNCTIONS FOR DAMAGE APPLICATION
// ============================================================================

// Apply damage with substitute logic
// Returns actual damage dealt to the Pokémon (0 if absorbed by substitute)
static inline int apply_damage_with_substitute(BattlePokemon *defender,
                                               int damage) {
  if (defender->substitute_hp > 0) {
    // Damage goes to substitute first
    if (damage >= defender->substitute_hp) {
      int overflow = damage - defender->substitute_hp;
      DLOG("%s's substitute broke!", get_pokemon_name(defender->pokemon->id));
      defender->substitute_hp = 0;
      // In Gen 1, overflow damage doesn't carry through
      return 0;
    } else {
      defender->substitute_hp -= damage;
      DLOG("%s's substitute took the hit! (%d HP remaining)",
           get_pokemon_name(defender->pokemon->id),
           defender->substitute_hp);
      return 0;
    }
  }
  // No substitute, damage goes directly to Pokémon
  defender->pokemon->hp -= damage;
  defender->pokemon->hp = max(defender->pokemon->hp, 0);
  return damage;
}

// ============================================================================
// MOVE EFFECT FUNCTIONS (Alphabetically Sorted)
// ============================================================================

// Acid - In Gen 1, 33.2% chance to lower defender's Defense (not poison)
void apply_acid(Battle *battle,
                BattlePokemon *attacker,
                BattlePokemon *defender) {
  if (defender->mist) {
    DLOG("%s is protected by Mist!", get_pokemon_name(defender->pokemon->id));
    return;
  }
  if (rand() % 256 < 85) {  // 85/256 ≈ 33.2%
    defender->stat_mods.defense = max(defender->stat_mods.defense - 1, -6);
    DLOG("%s's Defense fell!", get_pokemon_name(defender->pokemon->id));
  }
}

// Aurora Beam - 33.2% chance to lower defender's Attack by 1 stage
void apply_aurora_beam(Battle *battle,
                       BattlePokemon *attacker,
                       BattlePokemon *defender) {
  if (rand() % 256 < 85) {  // 85/256 ≈ 33.2%
    defender->stat_mods.attack = max(defender->stat_mods.attack - 1, -6);
    DLOG("%s's Attack fell!", get_pokemon_name(defender->pokemon->id));
  }
}

// Bide - User endures attacks for 2-3 turns, then strikes back with double
// damage
void apply_bide(Battle *battle,
                BattlePokemon *attacker,
                BattlePokemon *defender) {
  // First turn bide is used
  if (attacker->recharge_counter == 0) {
    // Either 2 or 3 moves
    attacker->recharge_len = rand() % 2 + 2;
    return;
  }
  if (attacker->recharge_counter == attacker->recharge_len) {
    // Bide is fully charged

    // This means that there should be a post attack phase where fainted pokemon
    // are swapped, And the queue is cleaned up.
    defender->pokemon->hp -= attacker->dmg_counter * 2;
    DLOG("Bide triggered!");
    attacker->recharge_counter = 0;
    attacker->recharge_len = 0;
    attacker->dmg_counter = 0;
    return;
  }
  attacker->recharge_counter++;
}

// Bind - Traps opponent for 2-5 turns, dealing damage each turn
void apply_bind(Battle *battle,
                BattlePokemon *attacker,
                BattlePokemon *defender) {
  // Calculate recharge counter
  // The number of turns is
  // 2 - 37.5%
  // 3 - 37.5%
  // 4 - 12.5%
  // 5 - 12.5%
  if (attacker->recharge_counter == 0) {
    int rand_val = rand() % 8;
    if (rand_val < 3) {
      attacker->recharge_len = 2;
      defender->recharge_len = 2;
    } else if (rand_val < 6) {
      attacker->recharge_len = 3;
      defender->recharge_len = 3;
    } else if (rand_val == 6) {
      attacker->recharge_len = 4;
      defender->recharge_len = 4;
    } else {
      attacker->recharge_len = 5;
      defender->recharge_len = 5;
    }

    // The applied damage for the move is constant for the following turns
  }

  if (attacker->recharge_counter == attacker->recharge_len) {
    // Bind has fully charged
    attacker->recharge_counter = 0;
    attacker->recharge_len = 0;
    defender->recharge_counter = 0;
    defender->recharge_len = 0;
    DLOG("Bind ended!");
    return;
  }

  attacker->recharge_counter++;
  defender->recharge_counter++;
}

// Bite - 10% chance to make the defender flinch (Gen 1)
void apply_bite(Battle *battle,
                BattlePokemon *attacker,
                BattlePokemon *defender) {
  if (rand() % 256 < 26) {  // 26/256 ≈ 10.2%
    defender->flinch = 1;
    DLOG("%s flinched!", get_pokemon_name(defender->pokemon->id));
  }
}

// Bubble - 33.2% chance to lower defender's Speed by 1 stage
void apply_bubble(Battle *battle,
                  BattlePokemon *attacker,
                  BattlePokemon *defender) {
  if (rand() % 256 < 85) {  // 85/256 ≈ 33.2%
    defender->stat_mods.speed = max(defender->stat_mods.speed - 1, -6);
    DLOG("%s's Speed fell!", get_pokemon_name(defender->pokemon->id));
  }
}

// Bubble Beam - 33.2% chance to lower defender's Speed by 1 stage
void apply_bubble_beam(Battle *battle,
                       BattlePokemon *attacker,
                       BattlePokemon *defender) {
  if (rand() % 256 < 85) {  // 85/256 ≈ 33.2%
    defender->stat_mods.speed = max(defender->stat_mods.speed - 1, -6);
    DLOG("%s's Speed fell!", get_pokemon_name(defender->pokemon->id));
  }
}

// Constrict - 33.2% chance to lower defender's Speed by 1 stage
void apply_constrict(Battle *battle,
                     BattlePokemon *attacker,
                     BattlePokemon *defender) {
  if (rand() % 256 < 85) {  // 85/256 ≈ 33.2%
    defender->stat_mods.speed = max(defender->stat_mods.speed - 1, -6);
    DLOG("%s's Speed fell!", get_pokemon_name(defender->pokemon->id));
  }
}

// Conversion - Changes user's type to match defender's type
void apply_conversion(Battle *battle,
                      BattlePokemon *attacker,
                      BattlePokemon *defender) {
  attacker->type1 = defender->type1;
  attacker->type2 = defender->type2;
  DLOG("%s transformed its type!", get_pokemon_name(attacker->pokemon->id));
}

// Counter - Returns double the damage from the last physical attack received
void apply_counter(Battle *battle,
                   BattlePokemon *attacker,
                   BattlePokemon *defender) {
  // Counter does double the damage received from the last physical attack
  // Only if the last attack was physical
  if (attacker->dmg_counter > 0 &&
      battle->lastMove->category == PHYSICAL_MOVE_CATEGORY) {
    defender->dmg_counter *= 2;
    attacker->pokemon->hp -= defender->dmg_counter;
    DLOG("Counter triggered!");
    defender->dmg_counter = 0;
  }
}

// Dig - Two-turn move: digs underground on turn 1 (invulnerable), attacks on
// turn 2
void apply_dig(Battle *battle,
               BattlePokemon *attacker,
               BattlePokemon *defender) {
  if (attacker->recharge_counter == 0) {
    attacker->recharge_len = 1;
    DLOG("%s burrowed underground!", get_pokemon_name(attacker->pokemon->id));
  } else {
    attacker->recharge_counter = 0;
    attacker->recharge_len = 0;
  }
}

// Disable - Prevents the last move (?) used by opponent from being used for 1-7
void apply_disable(Battle *battle,
                   BattlePokemon *attacker,
                   BattlePokemon *defender) {
  DLOG("Disable effect not fully implemented yet");
  if (defender->last_used == NULL) {
    return;
  }
  defender->disabled_index = rand() % 4;
  defender->disabled_count = rand() % 8;  // 0-7 turns
}

// Dizzy Punch - 20% chance to confuse the defender
void apply_dizzy_punch(Battle *battle,
                       BattlePokemon *attacker,
                       BattlePokemon *defender) {
  if (rand() % 256 < 51) {  // 51/256 ≈ 19.9%
    defender->confusion_counter = 1;
    DLOG("%s became confused!", get_pokemon_name(defender->pokemon->id));
  }
}

// Double Edge - Recoil damage equal to 1/4 of damage dealt
void apply_double_edge(Battle *battle,
                       BattlePokemon *attacker,
                       BattlePokemon *defender) {
  // Recoil damage is 1/4 of damage dealt
  int recoil = battle->lastDamage / 4;
  attacker->pokemon->hp -= recoil;
  DLOG("%s took %d recoil damage!",
       get_pokemon_name(attacker->pokemon->id),
       recoil);
}

// Explosion - User faints after using this move (250 base power)
void apply_explosion(Battle *battle,
                     BattlePokemon *attacker,
                     BattlePokemon *defender) {
  attacker->pokemon->hp = 0;
  DLOG("%s fainted from explosion!", get_pokemon_name(attacker->pokemon->id));
}

// Fire Blast - In Gen 1, 30% chance to burn the defender
void apply_fire_blast(Battle *battle,
                      BattlePokemon *attacker,
                      BattlePokemon *defender) {
  if (rand() % 256 < 77) {  // 77/256 ≈ 30.1%
    defender->pokemon->status.burn = 1;
    DLOG("%s was burned!", get_pokemon_name(defender->pokemon->id));
  }
}

// Fissure - One-hit KO move (if it hits)
// Gen 1 OHKO accuracy: (attacker_level - defender_level + 30)%
// Fails if attacker level < defender level
void apply_fissure(Battle *battle,
                   BattlePokemon *attacker,
                   BattlePokemon *defender) {
  // OHKO moves break substitutes but don't affect the Pokémon behind it
  if (defender->substitute_hp > 0) {
    DLOG("%s's substitute broke!", get_pokemon_name(defender->pokemon->id));
    defender->substitute_hp = 0;
    return;
  }

  // Check OHKO accuracy formula
  if (attacker->pokemon->stats.level < defender->pokemon->stats.level) {
    DLOG("Fissure failed! (attacker level too low)");
    return;
  }

  int accuracy =
      attacker->pokemon->stats.level - defender->pokemon->stats.level + 30;
  if (rand() % 100 >= accuracy) {
    DLOG("Fissure missed!");
    return;
  }

  // In Gen 1, if Fissure hits, it's an instant KO
  defender->pokemon->hp = 0;
  DLOG("%s was knocked out by Fissure!",
       get_pokemon_name(defender->pokemon->id));
}  // Fly - Two-turn move: flies up on turn 1 (invulnerable), attacks on turn 2
void apply_fly(Battle *battle,
               BattlePokemon *attacker,
               BattlePokemon *defender) {
  if (attacker->recharge_counter == 0) {
    attacker->recharge_len = 1;
    DLOG("%s flew up high!", get_pokemon_name(attacker->pokemon->id));
  } else {
    attacker->recharge_counter = 0;
    attacker->recharge_len = 0;
  }
}

// Glare - Paralyzes the defender
void apply_glare(Battle *battle,
                 BattlePokemon *attacker,
                 BattlePokemon *defender) {
  defender->pokemon->status.paralyzed = 1;
  DLOG("%s's paralyzed status was raised!",
       get_pokemon_name(defender->pokemon->id));
}

// Growl - Lowers defender's Attack by 1 stage
void apply_growl(Battle *battle,
                 BattlePokemon *attacker,
                 BattlePokemon *defender) {
  if (defender->mist) {
    DLOG("%s is protected by Mist!", get_pokemon_name(defender->pokemon->id));
    return;
  }
  DLOG("Applying Growl");
  defender->stat_mods.attack = max(defender->stat_mods.attack - 1, -6);
  DLOG("%s's Attack fell!", get_pokemon_name(defender->pokemon->id));
}

// Guillotine - One-hit KO move (if it hits)
// Gen 1 OHKO accuracy: (attacker_level - defender_level + 30)%
// Fails if attacker level < defender level
void apply_guillotine(Battle *battle,
                      BattlePokemon *attacker,
                      BattlePokemon *defender) {
  // OHKO moves break substitutes but don't affect the Pokémon behind it
  if (defender->substitute_hp > 0) {
    DLOG("%s's substitute broke!", get_pokemon_name(defender->pokemon->id));
    defender->substitute_hp = 0;
    return;
  }

  // Check OHKO accuracy formula
  if (attacker->pokemon->stats.level < defender->pokemon->stats.level) {
    DLOG("Guillotine failed! (attacker level too low)");
    return;
  }

  int accuracy =
      attacker->pokemon->stats.level - defender->pokemon->stats.level + 30;
  if (rand() % 100 >= accuracy) {
    DLOG("Guillotine missed!");
    return;
  }

  // In Gen 1, if Guillotine hits, it's an instant KO
  defender->pokemon->hp = 0;
  DLOG("%s was knocked out by Guillotine!",
       get_pokemon_name(defender->pokemon->id));
}  // Haze - Resets all stat modifications for both Pokémon
void apply_haze(Battle *battle,
                BattlePokemon *attacker,
                BattlePokemon *defender) {
  reset_battle_pokemon(attacker);
  reset_battle_pokemon(defender);

  DLOG("All stat changes were eliminated!");
}

// Horn Drill - One-hit KO move (if it hits)
// Gen 1 OHKO accuracy: (attacker_level - defender_level + 30)%
// Fails if attacker level < defender level
void apply_horn_drill(Battle *battle,
                      BattlePokemon *attacker,
                      BattlePokemon *defender) {
  // Check OHKO accuracy formula
  if (attacker->pokemon->stats.level < defender->pokemon->stats.level) {
    DLOG("Horn Drill failed! (attacker level too low)");
    return;
  }

  int accuracy =
      attacker->pokemon->stats.level - defender->pokemon->stats.level + 30;
  if (rand() % 100 >= accuracy) {
    DLOG("Horn Drill missed!");
    return;
  }

  // In Gen 1, if Horn Drill hits, it's an instant KO
  if (defender->substitute_hp > 0) {
    defender->substitute_hp = 0;
    return;
  }
  defender->pokemon->hp = 0;
  DLOG("%s was knocked out by Horn Drill!",
       get_pokemon_name(defender->pokemon->id));
}

// Hyper Beam - Must recharge on the following turn
void apply_hyper_beam(Battle *battle,
                      BattlePokemon *attacker,
                      BattlePokemon *defender) {
  // Must recharge next turn
  attacker->recharge_counter = 1;
  attacker->recharge_len = 1;
  DLOG("%s must recharge!", get_pokemon_name(attacker->pokemon->id));
}

void apply_leech_seed(Battle *battle,
                      BattlePokemon *attacker,
                      BattlePokemon *defender) {
  DLOG("Leech Seed planted");
  defender->leech_seed = 1;
}

void apply_light_screen(Battle *battle,
                        BattlePokemon *attacker,
                        BattlePokemon *defender) {
  attacker->light_screen = 1;
  DLOG("Light Screen raised");
}

// Low Kick - In Gen 1, has 50 base power (30% flinch chance)
// Note: Weight-based mechanics were added in Gen 3
void apply_low_kick(Battle *battle,
                    BattlePokemon *attacker,
                    BattlePokemon *defender) {
  if (rand() % 256 < 77) {  // 77/256 ≈ 30.1%
    defender->flinch = 1;
    DLOG("%s flinched!", get_pokemon_name(defender->pokemon->id));
  }
}

void apply_mimic(Battle *battle,
                 BattlePokemon *attacker,
                 BattlePokemon *defender) {
  DLOG("Mimic used");
  // Find the index of mimic in the attacker
  int mimic_index = 0;
  for (int i = 0; i < 4; ++i) {
    if (attacker->pokemon->poke_moves[i].id == MIMIC_MOVE_ID) {
      mimic_index = i;
      break;
    }
  }
  // Copy a random move from the defender
  int random_move_index = rand() % 4;
  attacker->moves[mimic_index].id = defender->moves[random_move_index].id;
  attacker->moves[mimic_index].power = defender->moves[random_move_index].power;
  attacker->moves[mimic_index].accuracy =
      defender->moves[random_move_index].accuracy;
  attacker->moves[mimic_index].type = defender->moves[random_move_index].type;
  attacker->moves[mimic_index].category =
      defender->moves[random_move_index].category;
  attacker->moves[mimic_index].pp = defender->moves[random_move_index].pp;
  attacker->moves[mimic_index].movePtr =
      defender->moves[random_move_index].movePtr;
  attacker->moves[mimic_index].priority =
      defender->moves[random_move_index].priority;
}

// Minimize - Raises user's evasion by 1 stage
void apply_minimize(Battle *battle,
                    BattlePokemon *attacker,
                    BattlePokemon *defender) {
  attacker->stat_mods.evasion = max(attacker->stat_mods.evasion + 1, 6);
  DLOG("%s's evasion rose!", get_pokemon_name(attacker->pokemon->id));
}

void apply_mirror_move(Battle *battle,
                       BattlePokemon *attacker,
                       BattlePokemon *defender) {
  DLOG("Mirror Move used");
  attack(battle, attacker, defender, defender->last_used);
}

void apply_mist(Battle *battle,
                BattlePokemon *attacker,
                BattlePokemon *defender) {
  attacker->mist = 1;
  DLOG("Mist surrounded the team");
}

// Night Shade - Deals damage equal to user's level
void apply_night_shade(Battle *battle,
                       BattlePokemon *attacker,
                       BattlePokemon *defender) {
  int damage = attacker->pokemon->stats.level;
  int actual_damage = apply_damage_with_substitute(defender, damage);
  if (actual_damage > 0) {
    DLOG("%s took %d damage from Night Shade!",
         get_pokemon_name(defender->pokemon->id),
         damage);
  }
}

// Petal Dance - Attacks for 2-3 turns then becomes confused
void apply_petal_dance(Battle *battle,
                       BattlePokemon *attacker,
                       BattlePokemon *defender) {
  if (attacker->recharge_counter == 0) {
    attacker->recharge_len = rand() % 2 + 2;  // 2 or 3 turns
    DLOG("%s began dancing!", get_pokemon_name(attacker->pokemon->id));
  }

  if (attacker->recharge_counter == attacker->recharge_len) {
    attacker->confusion_counter = 1;
    attacker->recharge_counter = 0;
    attacker->recharge_len = 0;
    DLOG("%s became confused from fatigue!",
         get_pokemon_name(attacker->pokemon->id));
  } else {
    attacker->recharge_counter++;
  }
}

// Poison Sting - 30% chance to poison the defender
void apply_poison_sting(Battle *battle,
                        BattlePokemon *attacker,
                        BattlePokemon *defender) {
  if (rand() % 256 < 77) {  // 77/256 ≈ 30.1%
    defender->pokemon->status.poison = 1;
    DLOG("%s was poisoned!", get_pokemon_name(defender->pokemon->id));
  }
}

// Psychic - 33.2% chance to lower defender's Special Defense by 1 stage
void apply_psychic(Battle *battle,
                   BattlePokemon *attacker,
                   BattlePokemon *defender) {
  if (rand() % 256 < 85) {  // 85/256 ≈ 33.2%
    defender->stat_mods.specD = max(defender->stat_mods.specD - 1, -6);
    DLOG("%s's Special Defense fell!", get_pokemon_name(defender->pokemon->id));
  }
}

// Psywave - In Gen 1, deals random damage from 1 to (1.5 × user's level)
void apply_psywave(Battle *battle,
                   BattlePokemon *attacker,
                   BattlePokemon *defender) {
  // Gen 1: Random damage from 1 to (level × 1.5), rounded down
  int max_damage = (int)(1.5 * attacker->pokemon->stats.level);
  int damage = (rand() % max_damage) + 1;
  int actual_damage = apply_damage_with_substitute(defender, damage);
  if (actual_damage > 0) {
    DLOG("%s took %d damage from Psywave!",
         get_pokemon_name(defender->pokemon->id),
         damage);
  }
}

void apply_rage(Battle *battle,
                BattlePokemon *attacker,
                BattlePokemon *defender) {
  DLOG("%s is enraged!", get_pokemon_name(attacker->pokemon->id));
  // find rage
  for (int i = 0; i < 4; ++i) {
    if (attacker->moves[i].id == RAGE_MOVE_ID ||
        attacker->pokemon->poke_moves[i].id == RAGE_MOVE_ID) {
      attacker->rage = attacker->moves + i;
    }
  }
}

// Razor Wind - Two-turn move: charges on turn 1, attacks on turn 2
void apply_razor_wind(Battle *battle,
                      BattlePokemon *attacker,
                      BattlePokemon *defender) {
  if (attacker->recharge_counter == 0) {
    attacker->recharge_len = 1;
    DLOG("%s is whipping up a whirlwind!",
         get_pokemon_name(attacker->pokemon->id));
  } else {
    attacker->recharge_counter = 0;
    attacker->recharge_len = 0;
  }
}

// Recover - Heals 50% of max HP
void apply_recover(Battle *battle,
                   BattlePokemon *attacker,
                   BattlePokemon *defender) {
  int heal_amount = attacker->pokemon->max_hp / 2;
  attacker->pokemon->hp =
      (attacker->pokemon->hp + heal_amount > attacker->pokemon->max_hp)
          ? attacker->pokemon->max_hp
          : attacker->pokemon->hp + heal_amount;
  DLOG("%s recovered %d HP!",
       get_pokemon_name(attacker->pokemon->id),
       heal_amount);
}

void apply_reflect(Battle *battle,
                   BattlePokemon *attacker,
                   BattlePokemon *defender) {
  attacker->reflect = 1;
  DLOG("Reflect raised");
}

// Rest - Heals to full HP and sleeps for 2 turns
void apply_rest(Battle *battle,
                BattlePokemon *attacker,
                BattlePokemon *defender) {
  attacker->pokemon->hp = attacker->pokemon->max_hp;
  attacker->pokemon->status.sleep = 2;
  DLOG("%s went to sleep and restored HP!",
       get_pokemon_name(attacker->pokemon->id));
}

// Rock Slide - 30% chance to make the defender flinch
void apply_rock_slide(Battle *battle,
                      BattlePokemon *attacker,
                      BattlePokemon *defender) {
  if (rand() % 256 < 77) {  // 77/256 ≈ 30.1%
    defender->flinch = 1;
    DLOG("%s flinched!", get_pokemon_name(defender->pokemon->id));
  }
}

// Sand Attack - Lowers defender's accuracy by 1 stage
void apply_sand_attack(Battle *battle,
                       BattlePokemon *attacker,
                       BattlePokemon *defender) {
  if (defender->mist) {
    DLOG("%s is protected by Mist!", get_pokemon_name(defender->pokemon->id));
    return;
  }
  defender->stat_mods.accuracy = max(defender->stat_mods.accuracy - 1, -6);
  DLOG("%s's accuracy fell!", get_pokemon_name(defender->pokemon->id));
}

// Seismic Toss - Deals damage equal to user's level
void apply_seismic_toss(Battle *battle,
                        BattlePokemon *attacker,
                        BattlePokemon *defender) {
  int damage = attacker->pokemon->stats.level;
  int actual_damage = apply_damage_with_substitute(defender, damage);
  if (actual_damage > 0) {
    DLOG("%s took %d damage from Seismic Toss!",
         get_pokemon_name(defender->pokemon->id),
         damage);
  }
}

// Self Destruct - User faints after using this move (200 base power)
void apply_self_destruct(Battle *battle,
                         BattlePokemon *attacker,
                         BattlePokemon *defender) {
  attacker->pokemon->hp = 0;
  DLOG("%s fainted from self-destruct!",
       get_pokemon_name(attacker->pokemon->id));
}

// Skull Bash - Two-turn move: charges and raises Defense on turn 1, attacks on
// turn 2
void apply_skull_bash(Battle *battle,
                      BattlePokemon *attacker,
                      BattlePokemon *defender) {
  if (attacker->recharge_counter == 0) {
    attacker->recharge_len = 1;
    attacker->stat_mods.defense = (attacker->stat_mods.defense + 1 > 6)
                                      ? 6
                                      : attacker->stat_mods.defense + 1;
    DLOG("%s lowered its head!", get_pokemon_name(attacker->pokemon->id));
  } else {
    attacker->recharge_counter = 0;
    attacker->recharge_len = 0;
  }
}

// Sky Attack - Two-turn move: charges on turn 1, attacks on turn 2
void apply_sky_attack(Battle *battle,
                      BattlePokemon *attacker,
                      BattlePokemon *defender) {
  if (attacker->recharge_counter == 0) {
    attacker->recharge_len = 1;
    DLOG("%s is glowing!", get_pokemon_name(attacker->pokemon->id));
  } else {
    attacker->recharge_counter = 0;
    attacker->recharge_len = 0;
  }
}

// Soft Boiled - Heals 50% of max HP
void apply_soft_boiled(Battle *battle,
                       BattlePokemon *attacker,
                       BattlePokemon *defender) {
  int heal_amount = attacker->pokemon->max_hp / 2;
  attacker->pokemon->hp =
      (attacker->pokemon->hp + heal_amount > attacker->pokemon->max_hp)
          ? attacker->pokemon->max_hp
          : attacker->pokemon->hp + heal_amount;
  DLOG("%s recovered %d HP!",
       get_pokemon_name(attacker->pokemon->id),
       heal_amount);
}

// Solar Beam - Two-turn move: charges on turn 1, attacks on turn 2
void apply_solar_beam(Battle *battle,
                      BattlePokemon *attacker,
                      BattlePokemon *defender) {
  if (attacker->recharge_counter == 0) {
    attacker->recharge_len = 1;
    DLOG("%s is absorbing light!", get_pokemon_name(attacker->pokemon->id));
  } else {
    attacker->recharge_counter = 0;
    attacker->recharge_len = 0;
  }
}

// Stomp - 30% chance to make the defender flinch
void apply_stomp(Battle *battle,
                 BattlePokemon *attacker,
                 BattlePokemon *defender) {
  if (rand() % 256 < 77) {  // 77/256 ≈ 30.1%
    defender->flinch = 1;
    DLOG("%s flinched!", get_pokemon_name(defender->pokemon->id));
  }
}

// Struggle - Recoil damage equal to 1/2 of user's max HP (used when out of PP)
void apply_struggle(Battle *battle,
                    BattlePokemon *attacker,
                    BattlePokemon *defender) {
  int recoil = attacker->pokemon->max_hp / 2;
  attacker->pokemon->hp -= recoil;
  DLOG("%s is hurt by recoil!", get_pokemon_name(attacker->pokemon->id));
}

// Substitute - Creates a substitute with 1/4 of user's max HP
void apply_substitute(Battle *battle,
                      BattlePokemon *attacker,
                      BattlePokemon *defender) {
  int substitute_cost = attacker->pokemon->max_hp / 4;

  // Check if user has enough HP to create substitute
  if (attacker->pokemon->hp <= substitute_cost) {
    DLOG("%s doesn't have enough HP to create a substitute!",
         get_pokemon_name(attacker->pokemon->id));
    return;
  }

  // Check if substitute already exists
  if (attacker->substitute_hp > 0) {
    DLOG("%s already has a substitute!",
         get_pokemon_name(attacker->pokemon->id));
    return;
  }

  // Create substitute
  attacker->pokemon->hp -= substitute_cost;
  attacker->substitute_hp = substitute_cost;
  DLOG("%s created a substitute!", get_pokemon_name(attacker->pokemon->id));
}

// Tail Whip - Lowers defender's Defense by 1 stage
void apply_tail_whip(Battle *battle,
                     BattlePokemon *attacker,
                     BattlePokemon *defender) {
  if (defender->mist) {
    DLOG("%s is protected by Mist!", get_pokemon_name(defender->pokemon->id));
    return;
  }
  defender->stat_mods.defense = max(defender->stat_mods.defense - 1, -6);
  DLOG("%s's Defense fell!", get_pokemon_name(defender->pokemon->id));
}

// Thrash - Attacks for 2-3 turns then becomes confused
void apply_thrash(Battle *battle,
                  BattlePokemon *attacker,
                  BattlePokemon *defender) {
  if (attacker->recharge_counter == 0) {
    attacker->recharge_len = rand() % 2 + 2;  // 2 or 3 turns
    DLOG("%s began thrashing!", get_pokemon_name(attacker->pokemon->id));
  }

  if (attacker->recharge_counter == attacker->recharge_len) {
    attacker->confusion_counter = 1;
    attacker->recharge_counter = 0;
    attacker->recharge_len = 0;
    DLOG("%s became confused from fatigue!",
         get_pokemon_name(attacker->pokemon->id));
  } else {
    attacker->recharge_counter++;
  }
}

// Thunder - In Gen 1, 10% chance to paralyze the defender
void apply_thunder(Battle *battle,
                   BattlePokemon *attacker,
                   BattlePokemon *defender) {
  if (rand() % 256 < 26) {  // 26/256 ≈ 10.2%
    defender->pokemon->status.paralyzed = 1;
    DLOG("%s was paralyzed!", get_pokemon_name(defender->pokemon->id));
  }
}

// Thunder Wave - Paralyzes the defender
void apply_thunder_wave(Battle *battle,
                        BattlePokemon *attacker,
                        BattlePokemon *defender) {
  defender->pokemon->status.paralyzed = 1;
  DLOG("%s's paralyzed status was raised!",
       get_pokemon_name(defender->pokemon->id));
}

// Toxic - Badly poisons the defender (damage increases each turn)
void apply_toxic(Battle *battle,
                 BattlePokemon *attacker,
                 BattlePokemon *defender) {
  defender->pokemon->status.poison = 1;
  defender->badly_poisoned_ctr = 1;
  DLOG("%s's poison status was raised!",
       get_pokemon_name(defender->pokemon->id));
}

// Transform - User transforms into the opponent, copying stats and moves
void apply_transform(Battle *battle,
                     BattlePokemon *attacker,
                     BattlePokemon *defender) {
  DLOG("%s transformed!", get_pokemon_name(attacker->pokemon->id));
  attacker->stats = defender->stats;
  for (int i = 0; i < 4; ++i) {
    attacker->moves[i].id = defender->pokemon->poke_moves[i].id;
    attacker->moves[i].power = defender->pokemon->poke_moves[i].power;
    attacker->moves[i].accuracy = defender->pokemon->poke_moves[i].accuracy;
    attacker->moves[i].type = defender->pokemon->poke_moves[i].type;
    attacker->moves[i].category = defender->pokemon->poke_moves[i].category;
    attacker->moves[i].pp = 5;
    attacker->moves[i].movePtr = defender->pokemon->poke_moves[i].movePtr;
    attacker->moves[i].priority = defender->pokemon->poke_moves[i].priority;
  }
  attacker->type1 = defender->type1;
  attacker->type2 = defender->type2;
}

// ============================================================================
// HELPER FUNCTIONS (Not directly mapped to moves in CSV)
// ============================================================================

// Bubble Beam Base - Helper function for speed-lowering moves
// Used as a base implementation for Bubble, Bubble Beam, and Constrict
void bubble_beam_base(Battle *battle,
                      BattlePokemon *attacker,
                      BattlePokemon *defender) {
  if (rand() % 256 < 85) {  // 85/256 ≈ 33.2%
    defender->stat_mods.speed = max(defender->stat_mods.speed - 1, -6);
    DLOG("%s's Speed fell!", get_pokemon_name(defender->pokemon->id));
  }
}

#endif