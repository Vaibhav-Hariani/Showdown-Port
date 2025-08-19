// switch.h: Handles switching logic and multiple switch types
#ifndef SWITCH_H
#define SWITCH_H

#include "log.h"
#include "pokemon.h"
#include "battle_queue.h"

typedef enum {
    hard_switch,
    forced_switch,
    double_switch,
    custom_switch
} switch_type;

// Main switch handler
void perform_switch_action(action *current_action, battlequeue *queue) {
    int target = current_action->action_d.switch_target;
    if (current_action->p->team[target].hp <= 0) {
        DLOG("Invalid switch: Target Pokémon has fainted.");
        return;
    }
    if (current_action->p->active_pokemon == target) {
        DLOG("Switch ignored: Pokémon already active.");
        return;
    }
    int old_active = current_action->p->active_pokemon;
    current_action->p->active_pokemon = target;
    DLOG("Player %d switched to %s!", current_action->player_num, get_pokemon_name(&current_action->p->team[target]));
    invalidate_queue(queue, old_active);
    // Extend here for other switch types as needed
}

#endif