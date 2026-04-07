#ifndef FAST_RNG_H
#define FAST_RNG_H

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#define SIM_THREAD_LOCAL _Thread_local
#elif defined(__GNUC__)
#define SIM_THREAD_LOCAL __thread
#else
#define SIM_THREAD_LOCAL
#endif

// Thread-local state avoids cross-thread contention while preserving speed.
static SIM_THREAD_LOCAL uint64_t sim_pcg_state = 0x853C49E6748FEA9BULL;
static SIM_THREAD_LOCAL uint64_t sim_pcg_inc = 0xDA3E39CB94B95BDBULL;

static inline void sim_srand(unsigned int seed) {
  // PCG seeding sequence: set odd increment and warm up once.
  sim_pcg_state = 0u;
  sim_pcg_inc = (((uint64_t)seed) << 1u) | 1u;
  // Advance once, add seed-derived state, advance again.
  uint64_t oldstate = sim_pcg_state;
  sim_pcg_state = oldstate * 6364136223846793005ULL + sim_pcg_inc;
  sim_pcg_state += (uint64_t)seed + 0x9E3779B97F4A7C15ULL;
  oldstate = sim_pcg_state;
  sim_pcg_state = oldstate * 6364136223846793005ULL + sim_pcg_inc;
}

static inline uint32_t sim_rand_u32(void) {
  uint64_t oldstate = sim_pcg_state;
  sim_pcg_state = oldstate * 6364136223846793005ULL + sim_pcg_inc;
  uint32_t xorshifted =
      (uint32_t)(((oldstate >> 18u) ^ oldstate) >> 27u);
  uint32_t rot = (uint32_t)(oldstate >> 59u);
  return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

static inline int sim_rand(void) {
  return (int)(sim_rand_u32() & 0x7FFFFFFFu);
}

static inline uint32_t sim_rand_bounded_u32(uint32_t bound) {
  if (bound == 0u) {
    return 0u;
  }
  return (uint32_t)(((uint64_t)sim_rand_u32() * (uint64_t)bound) >> 32);
}

static inline int sim_rand_bounded(int bound) {
  if (bound <= 0) {
    return 0;
  }
  return (int)sim_rand_bounded_u32((uint32_t)bound);
}

#endif
