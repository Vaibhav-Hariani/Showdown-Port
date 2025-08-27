#ifndef BATTLE_H
#define BATTLE_H

#include "basic_types.h"
#include "battle_queue.h"
#include "log.h"
#include "move.h"
#include "move_labels.h"
#include "pokedex_labels.h"
#include "pokemon.h"

Player* get_player(Battle* b, int i);
void end_step(Battle* b);

#endif