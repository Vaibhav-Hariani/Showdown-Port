float NEG_STAGE_MODIFIERS[] = {1.0,
                               66.0 / 100.0,
                               50.0 / 100.0,
                               40.0 / 100.0,
                               33.0 / 100.0,
                               28.0 / 100.0,
                               25.0 / 100.0};
float POS_STAGE_MODIFIERS[] = {1.0,
                               150.0 / 100.0,
                               200.0 / 100.0,
                               250.0 / 100.0,
                               300.0 / 100.0,
                               350.0 / 100.0,
                               400.0 / 100.0};

float get_stat_modifier(int stage) {
  return stage >= 0 ? POS_STAGE_MODIFIERS[stage] : NEG_STAGE_MODIFIERS[-stage];
}

float get_evasion_modifier(int stage) {
  return stage >= 0 ? NEG_STAGE_MODIFIERS[stage] : POS_STAGE_MODIFIERS[-stage];
}