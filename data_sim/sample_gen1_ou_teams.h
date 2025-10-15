// Sample RBY OU teams for testing and benchmarking
// Auto-generated from well-known competitive teams
#ifndef SAMPLE_GEN1_OU_TEAMS_H
#define SAMPLE_GEN1_OU_TEAMS_H

#include "poke_enum.h"
#include "generated_move_enum.h"

// Structure for a single Pokemon in a team
typedef struct {
  POKEDEX_IDS species;
  MOVE_IDS moves[4];
} TeamPokemon;

// Structure for a full team of 6 Pokemon
typedef struct {
  TeamPokemon pokemon[6];
} Team;

// Total number of sample teams: 6
#define NUM_SAMPLE_TEAMS 6

static const Team SAMPLE_GEN1_OU_TEAMS[] = {
  // Team 1
  {
    .pokemon = {
      {
        .species = TAUROS,
        .moves = {BODY_SLAM_MOVE_ID, HYPER_BEAM_MOVE_ID, EARTHQUAKE_MOVE_ID, BLIZZARD_MOVE_ID}
      },
      {
        .species = CHANSEY,
        .moves = {ICE_BEAM_MOVE_ID, THUNDER_WAVE_MOVE_ID, SOFT_BOILED_MOVE_ID, SEISMIC_TOSS_MOVE_ID}
      },
      {
        .species = EXEGGUTOR,
        .moves = {PSYCHIC_MOVE_ID, SLEEP_POWDER_MOVE_ID, STUN_SPORE_MOVE_ID, EXPLOSION_MOVE_ID}
      },
      {
        .species = SNORLAX,
        .moves = {BODY_SLAM_MOVE_ID, EARTHQUAKE_MOVE_ID, HYPER_BEAM_MOVE_ID, REST_MOVE_ID}
      },
      {
        .species = ALAKAZAM,
        .moves = {PSYCHIC_MOVE_ID, SEISMIC_TOSS_MOVE_ID, THUNDER_WAVE_MOVE_ID, RECOVER_MOVE_ID}
      },
      {
        .species = STARMIE,
        .moves = {SURF_MOVE_ID, BLIZZARD_MOVE_ID, THUNDER_WAVE_MOVE_ID, RECOVER_MOVE_ID}
      }
    }
  },
  // Team 2
  {
    .pokemon = {
      {
        .species = TAUROS,
        .moves = {BODY_SLAM_MOVE_ID, HYPER_BEAM_MOVE_ID, EARTHQUAKE_MOVE_ID, BLIZZARD_MOVE_ID}
      },
      {
        .species = SNORLAX,
        .moves = {BODY_SLAM_MOVE_ID, EARTHQUAKE_MOVE_ID, HYPER_BEAM_MOVE_ID, AMNESIA_MOVE_ID}
      },
      {
        .species = EXEGGUTOR,
        .moves = {PSYCHIC_MOVE_ID, SLEEP_POWDER_MOVE_ID, EXPLOSION_MOVE_ID, MEGA_DRAIN_MOVE_ID}
      },
      {
        .species = CHANSEY,
        .moves = {ICE_BEAM_MOVE_ID, THUNDER_WAVE_MOVE_ID, SOFT_BOILED_MOVE_ID, THUNDERBOLT_MOVE_ID}
      },
      {
        .species = ALAKAZAM,
        .moves = {PSYCHIC_MOVE_ID, THUNDER_WAVE_MOVE_ID, RECOVER_MOVE_ID, REFLECT_MOVE_ID}
      },
      {
        .species = JYNX,
        .moves = {BLIZZARD_MOVE_ID, PSYCHIC_MOVE_ID, LOVELY_KISS_MOVE_ID, REST_MOVE_ID}
      }
    }
  },
  // Team 3
  {
    .pokemon = {
      {
        .species = CHANSEY,
        .moves = {ICE_BEAM_MOVE_ID, THUNDER_WAVE_MOVE_ID, SOFT_BOILED_MOVE_ID, SEISMIC_TOSS_MOVE_ID}
      },
      {
        .species = SNORLAX,
        .moves = {BODY_SLAM_MOVE_ID, EARTHQUAKE_MOVE_ID, REST_MOVE_ID, AMNESIA_MOVE_ID}
      },
      {
        .species = EXEGGUTOR,
        .moves = {PSYCHIC_MOVE_ID, SLEEP_POWDER_MOVE_ID, STUN_SPORE_MOVE_ID, MEGA_DRAIN_MOVE_ID}
      },
      {
        .species = ALAKAZAM,
        .moves = {PSYCHIC_MOVE_ID, RECOVER_MOVE_ID, THUNDER_WAVE_MOVE_ID, SEISMIC_TOSS_MOVE_ID}
      },
      {
        .species = STARMIE,
        .moves = {SURF_MOVE_ID, BLIZZARD_MOVE_ID, THUNDER_WAVE_MOVE_ID, RECOVER_MOVE_ID}
      },
      {
        .species = RHYDON,
        .moves = {EARTHQUAKE_MOVE_ID, ROCK_SLIDE_MOVE_ID, BODY_SLAM_MOVE_ID, SUBSTITUTE_MOVE_ID}
      }
    }
  },
  // Team 4
  {
    .pokemon = {
      {
        .species = TAUROS,
        .moves = {BODY_SLAM_MOVE_ID, HYPER_BEAM_MOVE_ID, EARTHQUAKE_MOVE_ID, BLIZZARD_MOVE_ID}
      },
      {
        .species = SNORLAX,
        .moves = {BODY_SLAM_MOVE_ID, HYPER_BEAM_MOVE_ID, EARTHQUAKE_MOVE_ID, SELF_DESTRUCT_MOVE_ID}
      },
      {
        .species = EXEGGUTOR,
        .moves = {PSYCHIC_MOVE_ID, SLEEP_POWDER_MOVE_ID, EXPLOSION_MOVE_ID, DOUBLE_EDGE_MOVE_ID}
      },
      {
        .species = ALAKAZAM,
        .moves = {PSYCHIC_MOVE_ID, THUNDER_WAVE_MOVE_ID, REFLECT_MOVE_ID, SEISMIC_TOSS_MOVE_ID}
      },
      {
        .species = GENGAR,
        .moves = {THUNDERBOLT_MOVE_ID, EXPLOSION_MOVE_ID, PSYCHIC_MOVE_ID, HYPNOSIS_MOVE_ID}
      },
      {
        .species = ZAPDOS,
        .moves = {THUNDERBOLT_MOVE_ID, DRILL_PECK_MOVE_ID, THUNDER_WAVE_MOVE_ID, AGILITY_MOVE_ID}
      }
    }
  },
  // Team 5
  {
    .pokemon = {
      {
        .species = TAUROS,
        .moves = {BODY_SLAM_MOVE_ID, HYPER_BEAM_MOVE_ID, EARTHQUAKE_MOVE_ID, BLIZZARD_MOVE_ID}
      },
      {
        .species = SNORLAX,
        .moves = {BODY_SLAM_MOVE_ID, EARTHQUAKE_MOVE_ID, HYPER_BEAM_MOVE_ID, REST_MOVE_ID}
      },
      {
        .species = RHYDON,
        .moves = {EARTHQUAKE_MOVE_ID, ROCK_SLIDE_MOVE_ID, BODY_SLAM_MOVE_ID, SUBSTITUTE_MOVE_ID}
      },
      {
        .species = EXEGGUTOR,
        .moves = {PSYCHIC_MOVE_ID, SLEEP_POWDER_MOVE_ID, STUN_SPORE_MOVE_ID, EXPLOSION_MOVE_ID}
      },
      {
        .species = CHANSEY,
        .moves = {ICE_BEAM_MOVE_ID, THUNDER_WAVE_MOVE_ID, SOFT_BOILED_MOVE_ID, SEISMIC_TOSS_MOVE_ID}
      },
      {
        .species = LAPRAS,
        .moves = {SURF_MOVE_ID, BLIZZARD_MOVE_ID, THUNDERBOLT_MOVE_ID, BODY_SLAM_MOVE_ID}
      }
    }
  },
  // Team 6
  {
    .pokemon = {
      {
        .species = ALAKAZAM,
        .moves = {PSYCHIC_MOVE_ID, THUNDER_WAVE_MOVE_ID, RECOVER_MOVE_ID, SEISMIC_TOSS_MOVE_ID}
      },
      {
        .species = CHANSEY,
        .moves = {ICE_BEAM_MOVE_ID, THUNDERBOLT_MOVE_ID, SOFT_BOILED_MOVE_ID, THUNDER_WAVE_MOVE_ID}
      },
      {
        .species = EXEGGUTOR,
        .moves = {PSYCHIC_MOVE_ID, SLEEP_POWDER_MOVE_ID, DOUBLE_EDGE_MOVE_ID, EXPLOSION_MOVE_ID}
      },
      {
        .species = STARMIE,
        .moves = {SURF_MOVE_ID, BLIZZARD_MOVE_ID, THUNDER_WAVE_MOVE_ID, RECOVER_MOVE_ID}
      },
      {
        .species = JYNX,
        .moves = {BLIZZARD_MOVE_ID, PSYCHIC_MOVE_ID, LOVELY_KISS_MOVE_ID, REST_MOVE_ID}
      },
      {
        .species = TAUROS,
        .moves = {BODY_SLAM_MOVE_ID, HYPER_BEAM_MOVE_ID, EARTHQUAKE_MOVE_ID, BLIZZARD_MOVE_ID}
      }
    }
  }
};

#endif // SAMPLE_GEN1_OU_TEAMS_H