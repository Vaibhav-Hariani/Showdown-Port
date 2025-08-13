#ifndef POKEMON_H
#define POKEMON_H

#include <math.h>
#include <stdbool.h>
#include <stdint.h>

#include "effectiveness.h"

// TODO: Split up based on pokemon and move.
// currently it's a circular dependency.
// We can break it up with forward declarations, but this is easier for now.

struct Gen1PokedexEntry {
  uint8_t species;
  Types type1;
  Types type2;
} typedef Gen1PokedexEntry;

struct Gen1BaseStats {
  uint8_t hp;
  uint8_t attack;
  uint8_t defense;
  uint8_t speed;
  uint8_t special;
} typedef Gen1BaseStats;

struct Gen1StatExperience {
  uint16_t hp;
  uint16_t attack;
  uint16_t defense;
  uint16_t speed;
  uint16_t special;
} typedef Gen1StatExperience;

struct Gen1DV {
  uint8_t attack : 4;
  uint8_t defense : 4;
  uint8_t speed : 4;
  uint8_t special : 4;
} typedef Gen1DV;

struct Gen1Status {
  uint8_t paralyzed : 1;
  uint8_t burn : 1;
  uint8_t freeze : 1;
  uint8_t poison : 1;
  // sleep is a 3-bit value, 0 is no sleep, 1-7 is turns asleep
  uint8_t sleep : 3;
} typedef Gen1Status;

// Stats can be modified with a value between -6 and +6
// 0 is no change, -1 is -12.5%, +1 is +12.5%
struct Gen1StatModifier {
  int8_t attack : 4;   // Attack
  int8_t defense : 4;  // Defense
  // for gen 1, special attack and special defense are combined
  int8_t special : 4;   // Special Attack
  int8_t speed : 4;     // Speed
  int8_t accuracy : 4;  // Accuracy
  int8_t evasion : 4;   // Evasion
} typedef Gen1StatModifier;

enum MoveCategory {
  PHYSICAL_MOVE_CATEGORY = 0,
  SPECIAL_MOVE_CATEGORY,
  STATUS_MOVE_CATEGORY,
} typedef MoveCategory;

struct BattlePokemon;

struct Move {
  // move_id may look useless, but is useful for getting the move name
  uint16_t move_id;
  Types type;
  MoveCategory category;
  uint8_t pp;
  uint16_t power;
  float accuracy;  // if 2, then infinite accuracy
  uint8_t priority;
  // This would be a lot easier in C++, but we were asked to use C, so here we
  // are
  void (*effect)(struct BattlePokemon *attacker,
                 struct Move *attacker_move,
                 struct BattlePokemon *defender,
                 struct Move *defender_move);

} typedef Move;

// Built off of all the above
struct BattlePokemon {
  Gen1PokedexEntry entry;
  int16_t max_hp;  // int is convenient for dealing with negative values
  int16_t hp;
  uint8_t level;
  // The following are all derived from base stats, IVs, EVs, and level
  uint16_t attack;
  uint16_t defense;
  uint16_t speed;
  uint16_t special;
  uint16_t accuracy;
  uint16_t evasion;
  float crit_rate;

  // Corresponds to pokedex entry
  Move moves[4];  // Moves the Pokemon knows
  Gen1Status status;
  Gen1StatModifier stat_mods;

  // various transitive battle states
  uint8_t toxic_count;
  uint8_t bide_count;
  uint8_t bide_damage;
  bool flinch;

} typedef BattlePokemon;

void apply_move(BattlePokemon *attacker,
                int attacker_move_index,
                BattlePokemon *defender,
                int defender_move_index) {
  // TODO: Apply PP decrease
  // TODO: Apply accuracy
  // TODO: Check for ongoing effects
  attacker->moves[attacker_move_index].effect(
      attacker,
      &attacker->moves[attacker_move_index],
      defender,
      &defender->moves[defender_move_index]);
}

void build_pokemon(BattlePokemon *p,
                   Gen1PokedexEntry entry,
                   Gen1BaseStats base_stats,
                   Gen1DV dvs,
                   Gen1StatExperience evs,
                   uint8_t level,
                   Move moves[4]) {
  p->entry = entry;
  p->level = level;

  // STAT = int(((BaseStat + DV)*2+StatPoint)*Level/100)+E
  // where E = Level + 10 for HP
  // E = 5 for any other stat.
  // And StatPoint = int((SQRT(StatExp-1)+1)/4)

  int hp_dv = ((dvs.attack & 1) << 3) | ((dvs.defense & 1) << 2) |
              ((dvs.speed & 1) << 1) | (dvs.special & 1);
  int hp_stat_point = (int)((sqrt(evs.hp - 1) + 1) / 4);

  p->max_hp =
      ((base_stats.hp + hp_dv) * 2 + hp_stat_point) * level / 100 + level + 10;
  p->hp = p->max_hp;

  int attack_stat_point = (int)((sqrt(evs.attack - 1) + 1) / 4);
  p->attack =
      ((base_stats.attack + dvs.attack) * 2 + attack_stat_point) * level / 100 +
      5;

  int defense_stat_point = (int)((sqrt(evs.defense - 1) + 1) / 4);
  p->defense = ((base_stats.defense + dvs.defense) * 2 + defense_stat_point) *
                   level / 100 +
               5;

  int speed_stat_point = (int)((sqrt(evs.speed - 1) + 1) / 4);
  p->speed =
      ((base_stats.speed + dvs.speed) * 2 + speed_stat_point) * level / 100 + 5;

  int special_stat_point = (int)((sqrt(evs.special - 1) + 1) / 4);
  p->special = ((base_stats.special + dvs.special) * 2 + special_stat_point) *
                   level / 100 +
               5;

  p->accuracy = 100;
  p->evasion = 100;
  p->crit_rate = (float)base_stats.speed * 100.0f / 512.0f;
  for (int i = 0; i < 4; i++) {
    p->moves[i] = moves[i];
  }
  p->status.paralyzed = 0;
  p->status.burn = 0;
  p->status.freeze = 0;
  p->status.poison = 0;
  p->status.sleep = 0;
  p->stat_mods.attack = 0;
  p->stat_mods.defense = 0;
  p->stat_mods.special = 0;
  p->stat_mods.speed = 0;
  p->stat_mods.accuracy = 0;
  p->stat_mods.evasion = 0;
  p->toxic_count = 0;
  p->bide_count = 0;
  p->bide_damage = 0;
  p->flinch = false;
}

#endif  // POKEMON_H