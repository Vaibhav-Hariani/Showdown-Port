// Effectively, the runner.
//  Sourced from battle.ts

#ifndef BATTLE_H
#define BATTLE_H
#include "move.h"
#include "pokemon.h"
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
#endif