#ifndef MOVE_DEX_H
#define MOVE_DEX_H

#include <stdbool.h>
#include <stdlib.h>

#include "battle.h"
#include "generated_move_enum.h"
#include "log.h"
#include "move.h"
#include "move_labels.h"
#include "pokemon.h"
#include "typing.h"
// TODO: Bind, Counter, Dig, Fly, Disable, Gust, etc.

// Move dex for Gen 1
// Moves are defined as functions, so this is a list of pointers to those
// functions Each move has a name, type, power, accuracy, and PP

// https://bulbapedia.bulbagarden.net/wiki/Damage
// It is expected that move is a pointer to the move within

inline int max(int a, int b) { return (a > b) ? a : b; }

void apply_acid(Battle *battle, BattlePokemon *attacker, Move *attacker_move,
                BattlePokemon *defender) {
  if (((float)rand()) / 100.0 < 33.2) {
    defender->pokemon->status.poison = 1;
  }
}

void apply_aurora_beam(Battle *battle, BattlePokemon *attacker,
                       Move *attacker_move, BattlePokemon *defender) {
  if (((float)rand()) / 100.0 < 33.2) {
    defender->stat_mods.attack = max(defender->stat_mods.attack - 1, -6);
  }
}

// technically, bubble, bubble beam, and constrict all have the same effect

void bubble_beam_base(Battle *battle, BattlePokemon *attacker,
                      Move *attacker_move, BattlePokemon *defender) {
  if (((float)rand()) / 100.0 < 33.2) {
    defender->stat_mods.speed = max(defender->stat_mods.speed - 1, -6);
  }
}

void apply_bubble(Battle *battle, BattlePokemon *attacker, Move *attacker_move,
                  BattlePokemon *defender) {
  if (((float)rand()) / 100.0 < 33.2) {
    defender->stat_mods.speed = max(defender->stat_mods.speed - 1, -6);
  }
}

void apply_bubble_beam(Battle *battle, BattlePokemon *attacker,
                       Move *attacker_move, BattlePokemon *defender) {
  if (((float)rand()) / 100.0 < 33.2) {
    defender->stat_mods.speed = max(defender->stat_mods.speed - 1, -6);
  }
}

void apply_constrict(Battle *battle, BattlePokemon *attacker,
                     Move *attacker_move, BattlePokemon *defender) {
  if (((float)rand()) / 100.0 < 33.2) {
    defender->stat_mods.speed = max(defender->stat_mods.speed - 1, -6);
  }
}

void apply_bide(Battle *battle, BattlePokemon *attacker, Move *attacker_move,
                BattlePokemon *defender) {
  // First turn bide is used
  if (attacker->recharge_counter == 0) {
    // Either 2 or 3 moves
    attacker->recharge_len = rand() % 2 + 2;
    return;
  }
  if (attacker->recharge_counter == attacker->recharge_len) {
    // Bide is fully charged

    // This means that there should be a post attack phase where fainted pokemon are swapped,
    // And the queue is cleaned up.
    defender->pokemon->hp -= attacker->dmg_counter * 2;
    DLOG("Bide triggered!");
    attacker->recharge_counter = 0;
    attacker->recharge_len = 0;
    attacker->dmg_counter = 0;
    return;
  }
  attacker->recharge_counter++;
}

void apply_bite(Battle *battle, BattlePokemon *attacker, Move *attacker_move,
                BattlePokemon *defender) {
  if (((float)rand()) / 100.0 < 10.0) {
    defender->flinch = 1;
  }
}

void apply_conversion(Battle *battle, BattlePokemon *attacker,
                      Move *attacker_move, BattlePokemon *defender) {
  attacker->type1 = defender->type1;
  attacker->type2 = defender->type2;
}

void apply_fire_blast(Battle *battle, BattlePokemon *attacker,
                      Move *attacker_move, BattlePokemon *defender) {
  if (((float)rand()) / 100.0 < 30.1) {
    defender->pokemon->status.burn = 1;
  }
}

void apply_glare(Battle *battle, BattlePokemon *attacker, Move *attacker_move,
                 Pokemon *defender) {
  defender->status.paralyzed = 1;
}

void apply_toxic(Battle *battle, BattlePokemon *attacker, Move *attacker_move,
                 BattlePokemon *defender) {
  defender->pokemon->status.poison = 1;
  defender->badly_poisoned_ctr = 1;
}

void apply_thunder_wave(Battle *battle, BattlePokemon *attacker,
                        Move *attacker_move, BattlePokemon *defender) {
  defender->pokemon->status.paralyzed = 1;
}

void apply_tail_whip(Battle *battle, BattlePokemon *attacker,
                     Move *attacker_move, BattlePokemon *defender) {
  DLOG("Applying Tail Whip");
  defender->stat_mods.defense = max(defender->stat_mods.defense - 1, -6);
}

void apply_growl(Battle *battle, BattlePokemon *attacker, Move *attacker_move,
                 BattlePokemon *defender) {
  DLOG("Applying Growl");
  defender->stat_mods.attack = max(defender->stat_mods.attack - 1, -6);
}

#endif