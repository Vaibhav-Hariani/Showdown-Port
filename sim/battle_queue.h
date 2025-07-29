//Trying to re-implement the battle queue from showdown
#include "gamestate.h"

//Basic idea: player swings, moves collide (based on priority/speed), and then follow up triggers hit the stack
//This is less important for Gen1 but the design is important for resolving multiple triggers correctly.
// and I'm fleshing it out now.

//Likely will not need to dynamically allocate as these actions 
//will live and die on the stack when moves are done.

struct STR_MOVE_ACTION {
    void* move(pokemon*, pokemon*);

    //Enums should be used for mega/zmove/maxmoves
    // bool mega;
    // bool zmove;
    // bool maxmove;
    // Action source_Action: this is included in showdown but unsure about usage 
};

struct STR_SWITCH_ACTION {};

enum ENUM_ACTIONS {
    move_action,
    switch_action,
    team_action,
    field_action
} typedef action_types;

union UN_ACTIONS {
    struct STR_MOVE_ACTION m;
    struct STR_SWITCH_ACTION s;
    // struct STR_TEAM_ACTION t;
    // struct STR_FIELD_ACTION m;
} typedef action_u;

struct STR_ACTION {
    int order;
    //I believe this is unneeded for switching
    int priority;
    // Speed can probably be pulled from the origin pokemon
    int speed;
    int origLoc; // (index of the pokemon making the move)
    // Using this as a replacement for 

    int targetLoc; // (index of the pokemon being targetted)
    //This is only used for more complex effects, not determining the end target
    // pokemon initiate;

    // pokemon OriginalTarget;
    action_u action;
    int action_type;    
} typedef action;