// Fixed-point stat modifiers: 256 = 1.0x multiplier
// This eliminates float operations on the hot path
static const uint16_t NEG_STAGE_MODIFIERS[] = {
    256,  // 1.00x
    169,  // 0.66x (66/100 * 256)
    128,  // 0.50x (50/100 * 256)
    102,  // 0.40x (40/100 * 256)
    84,   // 0.33x (33/100 * 256)
    72,   // 0.28x (28/100 * 256)
    64    // 0.25x (25/100 * 256)
};

static const uint16_t POS_STAGE_MODIFIERS[] = {
    256,  // 1.00x
    384,  // 1.50x (150/100 * 256)
    512,  // 2.00x (200/100 * 256)
    640,  // 2.50x (250/100 * 256)
    768,  // 3.00x (300/100 * 256)
    896,  // 3.50x (350/100 * 256)
    1024  // 4.00x (400/100 * 256)
};

// Returns fixed-point multiplier (256 = 1.0x)
// Caller must divide result by 256 after multiplication
static inline uint16_t get_stat_modifier(int stage) {
  return stage >= 0 ? POS_STAGE_MODIFIERS[stage] : NEG_STAGE_MODIFIERS[-stage];
}

static inline uint16_t get_evasion_modifier(int stage) {
  return stage >= 0 ? NEG_STAGE_MODIFIERS[stage] : POS_STAGE_MODIFIERS[-stage];
}