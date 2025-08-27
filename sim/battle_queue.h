#ifndef BATTLE_QUEUE_H
#define BATTLE_QUEUE_H

// Trying to re-implement the battle queue from showdown

#include "basic_types.h"

/**
Basic idea: player swings, moves collide (based on priority/speed), and then
follow up triggers hit the stack. Less important for Gen1.
**/

void invalidate_queue(int completed, BattleQueue* queue);
int eval_queue(Battle* b);

#endif
