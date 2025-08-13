#ifndef MOVE_DEX_H
#define MOVE_DEX_H

#include <stdbool.h>
#include <stdlib.h>

#include "effectiveness.h"
#include "generated_move_enum.h"
#include "log.h"
#include "move_labels.h"
#include "pokemon.h"

// TODO: Bind, Counter, Dig, Fly, Disable, Gust, etc.

// Move dex for Gen 1
// Moves are defined as functions, so this is a list of pointers to those
// functions Each move has a name, type, power, accuracy, and PP

// https://bulbapedia.bulbagarden.net/wiki/Damage
// It is expected that move is a pointer to the move within

inline int max(int a, int b) { return (a > b) ? a : b; }

void apply_damaging_attack(BattlePokemon *attacker,
                           Move *attacker_move,
                           BattlePokemon *defender,
                           __attribute__((unused)) Move *defender_move) {
  float crit_rate = attacker->crit_rate;
  if (attacker_move->move_id == KARATE_CHOP_MOVE_ID ||
      attacker_move->move_id == RAZOR_LEAF_MOVE_ID ||
      attacker_move->move_id == SLASH_MOVE_ID ||
      attacker_move->move_id == CRABHAMMER_MOVE_ID) {
    crit_rate *= 8;
  }
  float critical = (crit_rate >= ((float)(rand() % 100))) ? 2.0 : 1.0;
  float stab = 1.0;
  if (attacker->entry.type1 == attacker_move->type ||
      attacker->entry.type2 == attacker_move->type) {
    stab = 1.5;
  }

  float type_1 =
      EFFECTIVENESS_TO_FLOAT[EffectivenessTable[attacker_move->type]
                                               [defender->entry.type1]];
  float type_2 =
      EFFECTIVENESS_TO_FLOAT[EffectivenessTable[attacker_move->type]
                                               [defender->entry.type2]];

  // random number between 217 and 255 (inclusive) followed by integer division
  // by 255
  int random = (rand() % 39 + 217);

  float power = attacker_move->power;
  if (attacker_move->move_id == BIDE_MOVE_ID && attacker->bide_count == 3) {
    power = attacker->bide_damage * 2;
  }

  int32_t damage =
      (int)(((((2.0f * attacker->level * critical) / 5.0f) + 2.0f) * power *
                 ((float)attacker->attack / (float)defender->defense) / 50.0f +
             2.0f) *
            stab * type_1 * type_2);
  if (damage > 1) {
    damage = damage * random;
    damage = damage / 255;
  }

  DLOG(
      "Damage calculation for %s: (damage, level, critical, crit_rate, power, "
      "STAB, "
      "type1, "
      "type2, "
      "random): "
      "(%d, %d, %.1f, %.3f, %.1f, %.1f, %.2f, %.2f, %d)",
      MOVE_LABELS[attacker_move->move_id],
      damage,
      attacker->level,
      critical,
      attacker->crit_rate,
      power,
      stab,
      type_1,
      type_2,
      random);
  defender->hp -= damage;
  if (defender->hp < 0) {
    defender->hp = 0;
  }

  if (defender->bide_count > 0) {
    defender->bide_damage += (int16_t)damage;
  }

  if (attacker_move->move_id == DOUBLE_EDGE_MOVE_ID) {
    attacker->hp -= (int16_t)((float)damage / 4.0);
    if (attacker->hp < 0) {
      attacker->hp = 0;
    }
  }

  if (attacker_move->move_id == SELF_DESTRUCT_MOVE_ID ||
      attacker_move->move_id == EXPLOSION_MOVE_ID) {
    attacker->hp = 0;
  }
}

void apply_acid(BattlePokemon *attacker,
                Move *attacker_move,
                BattlePokemon *defender,
                Move *defender_move) {
  apply_damaging_attack(attacker, attacker_move, defender, defender_move);
  if (((float)rand()) / 100.0 < 33.2) {
    defender->status.poison = 1;
  }
}

void apply_aurora_beam(BattlePokemon *attacker,
                       Move *attacker_move,
                       BattlePokemon *defender,
                       Move *defender_move) {
  apply_damaging_attack(attacker, attacker_move, defender, defender_move);
  if (((float)rand()) / 100.0 < 33.2) {
    defender->stat_mods.attack = max(defender->stat_mods.attack - 1, -6);
  }
}

// technically, bubble, bubble beam, and constrict all have the same effect

void apply_bubble(BattlePokemon *attacker,
                  Move *attacker_move,
                  BattlePokemon *defender,
                  Move *defender_move) {
  apply_damaging_attack(attacker, attacker_move, defender, defender_move);
  if (((float)rand()) / 100.0 < 33.2) {
    defender->stat_mods.speed = max(defender->stat_mods.speed - 1, -6);
  }
}

void apply_bubble_beam(BattlePokemon *attacker,
                       Move *attacker_move,
                       BattlePokemon *defender,
                       Move *defender_move) {
  apply_damaging_attack(attacker, attacker_move, defender, defender_move);
  if (((float)rand()) / 100.0 < 33.2) {
    defender->stat_mods.speed = max(defender->stat_mods.speed - 1, -6);
  }
}

void apply_constrict(BattlePokemon *attacker,
                     Move *attacker_move,
                     BattlePokemon *defender,
                     Move *defender_move) {
  apply_damaging_attack(attacker, attacker_move, defender, defender_move);
  if (((float)rand()) / 100.0 < 33.2) {
    defender->stat_mods.speed = max(defender->stat_mods.speed - 1, -6);
  }
}

void apply_bide(BattlePokemon *attacker,
                Move *attacker_move,
                BattlePokemon *defender,
                Move *defender_move) {
  if (attacker->bide_count == 0) {
    attacker->bide_count += 1;
  }

  apply_damaging_attack(attacker, attacker_move, defender, defender_move);

  if (attacker->bide_count == 3) {
    attacker->bide_count = 0;
    attacker->bide_damage = 0;
  }
}

void apply_bite(BattlePokemon *attacker,
                Move *attacker_move,
                BattlePokemon *defender,
                Move *defender_move) {
  apply_damaging_attack(attacker, attacker_move, defender, defender_move);
  if (((float)rand()) / 100.0 < 10.0) {
    defender->flinch = true;
  }
}

void apply_conversion(BattlePokemon *attacker,
                      __attribute__((unused)) Move *attacker_move,
                      BattlePokemon *defender,
                      __attribute__((unused)) Move *defender_move) {
  attacker->entry.type1 = defender->entry.type1;
  attacker->entry.type2 = defender->entry.type2;
}

void apply_fire_blast(BattlePokemon *attacker,
                      Move *attacker_move,
                      BattlePokemon *defender,
                      Move *defender_move) {
  apply_damaging_attack(attacker, attacker_move, defender, defender_move);
  if (((float)rand()) / 100.0 < 30.1) {
    defender->status.burn = 1;
  }
}

void apply_glare(__attribute__((unused)) BattlePokemon *attacker,
                 __attribute__((unused)) Move *attacker_move,
                 BattlePokemon *defender,
                 __attribute__((unused)) Move *defender_move) {
  defender->status.paralyzed = 1;
}

void apply_toxic(__attribute__((unused)) BattlePokemon *attacker,
                 __attribute__((unused)) Move *attacker_move,
                 BattlePokemon *defender,
                 __attribute__((unused)) Move *defender_move) {
  defender->status.poison = 1;
  defender->toxic_count = 1;
}

void apply_thunder_wave(__attribute__((unused)) BattlePokemon *attacker,
                        __attribute__((unused)) Move *attacker_move,
                        BattlePokemon *defender,
                        __attribute__((unused)) Move *defender_move) {
  defender->status.paralyzed = 1;
}

void apply_tail_whip(__attribute__((unused)) BattlePokemon *attacker,
                     __attribute__((unused)) Move *attacker_move,
                     BattlePokemon *defender,
                     __attribute__((unused)) Move *defender_move) {
  DLOG("Applying Tail Whip");
  defender->stat_mods.defense = max(defender->stat_mods.defense - 1, -6);
}

void apply_growl(__attribute__((unused)) BattlePokemon *attacker,
                 __attribute__((unused)) Move *attacker_move,
                 BattlePokemon *defender,
                 __attribute__((unused)) Move *defender_move) {
  DLOG("Applying Growl");
  defender->stat_mods.attack = max(defender->stat_mods.attack - 1, -6);
}

#endif