#ifndef MOVE_H
#define MOVE_H

#include <stddef.h>

#include "basic_types.h"
#include "generated_move_enum.h"
#include "move_fwd.h"
#include "typing.h"

// Battle queue related enums and structs
enum ACTION_MODES { REGULAR, FAINTED };

union UN_ACTIONS {
  Move* m;
  int switch_target;
} typedef action_union;

// Function declarations only - implementations are in basic_types.h after
// struct definitions
int calculate_damage(BattlePokemon* attacker,
                     BattlePokemon* defender,
                     Move* used_move);
int pre_move_check(BattlePokemon* attacker, Move* used_move);
int attack(Battle* b,
           BattlePokemon* attacker,
           BattlePokemon* defender,
           Move* used_move);
int valid_move(Player* user, int move_index);
void print_move_info(Move* m);
int add_move_to_queue(Battle* battle,
                      Player* user,
                      Player* target,
                      int move_index);
// We do it this way to avoid a circular dependency
void reset_action(Action* a);
action_union get_action_action_d(Action* a);
void set_action_action_d(Action* a, action_union d);
action_types get_action_action_type(Action* a);
void set_action_action_type(Action* a, action_types type);
Player* get_action_user(Action* a);
void set_action_user(Action* a, Player* user);
Player* get_action_target(Action* a);
void set_action_target(Action* a, Player* target);
int get_action_order(Action* a);
void set_action_order(Action* a, int order);
int get_action_priority(Action* a);
void set_action_priority(Action* a, int priority);
int get_action_speed(Action* a);
void set_action_speed(Action* a, int speed);
int get_action_orig_loc(Action* a);
void set_action_orig_loc(Action* a, int origLoc);
size_t get_action_size();

#endif
