typedef enum {
  STATUS_BURN = 1 << 0,
  STATUS_FREEZE = 1 << 1,
  STATUS_PARALYSIS = 1 << 2,
  STATUS_POISON = 1 << 3,
  STATUS_SLEEP = 1 << 4
} StatusFlags;

struct STR_MODS {
  // Move/ability effects that modify stats temporarily: scales
  // power/defense/etc.
  int stat_mods[5];
  // Bitfield set by above
  int status_flags;
} typedef modifiers;

// Gets loaded when a pokemon is selected with HP and stats, based on base data
// + EVs and IVs
struct STR_STATS {
  int stats[5];

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

// Comparable to the showdown PokemonSet object, but with battle data.
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