// switch.h: Handles switching logic and multiple switch types
#ifndef SWITCH_H
#define SWITCH_H

#include "battle_queue.h"
#include "move.h"

int valid_switch(Player* cur, int target_loc);
void add_switch(Battle* b, Player* user, int target_loc, int type);
void perform_switch_action(Action* current_action);

#endif