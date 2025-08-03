struct STR_STAT_FLAGS{
  int paralyzed: 1;
  int burn: 1;
  int freeze: 1;
  int poision: 1;
  int sleep: 1;
};

struct STR_MODS {
  // Move/ability effects that modify stats temporarily: applies on base stats
  // Undetermined if this should be a percentage vs. actual val...
  int stat_mods[5];
  STR_STAT_FLAGS status;  
  // Bitfield set by above
} typedef modifiers;


// Gets loaded when a pokemon is selected 
// based on base data, selected EVs, and IVs
struct STR_STATS {
  unsigned char stats[5];
  // Future relevance
  //  bool gender;
  //  int item_id;
  //  int happiness;
  //  int ability_id;
  //  int nature_id;

  // > Gen7 tomfoolery
  // int hp_type;
  // int dmaxlevel;
  // int gmaxlvl;
  // int teratype;
} typedef poke_stats;

struct STR_MOVES {
  int move_id;
  // Moves are currently being defined as functions, each move obj
  //  is a pointer to said function.
  int *movePtr(pokemon *, pokemon *);
  int pp;
} typedef poke_moves;

// Comparable to the showdown PokemonSet object, but also storing battle data.
struct STR_POKE {
  // Corresponds to pokedex entry
  unsigned char id;
  poke_moves moves[4];
  // EVs and IVs combined with base stats for compression
  poke_stats stat;

  // Relevant to battle state
  modifiers mods;
  int hp;

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