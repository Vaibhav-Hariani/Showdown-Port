#define PARALYSIS 

typedef enum {
    STATUS_BURN      = 1 << 0,
    STATUS_FREEZE    = 1 << 1,
    STATUS_PARALYSIS = 1 << 2,
    STATUS_POISON    = 1 << 3,
    STATUS_SLEEP     = 1 << 4
} StatusFlags;

struct STR_MODS {
    //Move/ability effects that modify stats temporarily:
    int stat_mods[5];
    int status_flags;
} typedef modifiers;

//Gets loaded when a pokemon is selected with HP and stats, based on base data + EVs and IVs
struct STR_STATS {
    int stats[5];
    int hp;
    //Any additional stats relevant to certain moves: 
} typedef poke_stats;

struct STR_MOVES {
    int move_id;
    int pp;
} typedef poke_moves;


struct STR_POKE {
    // Corresponds to pokedex entry
    unsigned char id;
    poke_stats stat;
    modifiers mods;
    int hp;
    poke_moves moves[4];
    //Need to figure out how to do multi typing effectively
    char type_id;

} typedef pokemon;

struct STR_PLAYER {
    pokemon team[6];
    char active_pokemon;
} typedef player;

struct STR_FIELD {
    player p1;
    player p2;
    char cur_player;
 } typedef field;