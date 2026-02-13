"""
RBY OU Teams data - Python version of ou_teams.h
Teams are structured as a list of 6 Pokemon, each with a species name and 4 moves.
"""

# RBY OU sample teams
# Each team is a list of 6 dicts with 'species' and 'moves' keys
OU_TEAMS = [
    [
        {
            "species": "Starmie",
            "moves": ["Psychic", "Blizzard", "Thunder Wave", "Recover"],
        },
        {
            "species": "Exeggutor",
            "moves": ["Sleep Powder", "Psychic", "Double-Edge", "Explosion"],
        },
        {
            "species": "Alakazam",
            "moves": ["Psychic", "Seismic Toss", "Thunder Wave", "Recover"],
        },
        {
            "species": "Chansey",
            "moves": ["Ice Beam", "Thunderbolt", "Thunder Wave", "Soft-Boiled"],
        },
        {"species": "Snorlax", "moves": ["Body Slam", "Reflect", "Hyper Beam", "Rest"]},
        {
            "species": "Tauros",
            "moves": ["Body Slam", "Hyper Beam", "Blizzard", "Earthquake"],
        },
    ],
    [
        {"species": "Jynx", "moves": ["Lovely Kiss", "Psychic", "Blizzard", "Counter"]},
        {
            "species": "Exeggutor",
            "moves": ["Psychic", "Stun Spore", "Mega Drain", "Explosion"],
        },
        {
            "species": "Zapdos",
            "moves": ["Thunderbolt", "Drill Peck", "Thunder Wave", "Agility"],
        },
        {
            "species": "Chansey",
            "moves": ["Sing", "Seismic Toss", "Thunder Wave", "Soft-Boiled"],
        },
        {
            "species": "Snorlax",
            "moves": ["Body Slam", "Surf", "Counter", "Self-Destruct"],
        },
        {
            "species": "Tauros",
            "moves": ["Body Slam", "Earthquake", "Blizzard", "Hyper Beam"],
        },
    ],
    [
        {"species": "Jynx", "moves": ["Blizzard", "Body Slam", "Lovely Kiss", "Rest"]},
        {
            "species": "Exeggutor",
            "moves": ["Psychic", "Stun Spore", "Mega Drain", "Explosion"],
        },
        {
            "species": "Gengar",
            "moves": ["Psychic", "Thunderbolt", "Hypnosis", "Explosion"],
        },
        {
            "species": "Zapdos",
            "moves": ["Thunderbolt", "Drill Peck", "Thunder Wave", "Agility"],
        },
        {
            "species": "Snorlax",
            "moves": ["Body Slam", "Earthquake", "Self-Destruct", "Hyper Beam"],
        },
        {
            "species": "Tauros",
            "moves": ["Body Slam", "Earthquake", "Thunderbolt", "Hyper Beam"],
        },
    ],
    [
        {
            "species": "Gengar",
            "moves": ["Psychic", "Thunderbolt", "Hypnosis", "Explosion"],
        },
        {
            "species": "Exeggutor",
            "moves": ["Psychic", "Stun Spore", "Mega Drain", "Explosion"],
        },
        {
            "species": "Zapdos",
            "moves": ["Thunderbolt", "Drill Peck", "Thunder Wave", "Agility"],
        },
        {
            "species": "Chansey",
            "moves": ["Sing", "Seismic Toss", "Thunder Wave", "Soft-Boiled"],
        },
        {"species": "Snorlax", "moves": ["Body Slam", "Earthquake", "Reflect", "Rest"]},
        {
            "species": "Tauros",
            "moves": ["Body Slam", "Earthquake", "Blizzard", "Hyper Beam"],
        },
    ],
    [
        {
            "species": "Alakazam",
            "moves": ["Psychic", "Seismic Toss", "Thunder Wave", "Recover"],
        },
        {
            "species": "Exeggutor",
            "moves": ["Psychic", "Sleep Powder", "Mega Drain", "Explosion"],
        },
        {
            "species": "Starmie",
            "moves": ["Psychic", "Blizzard", "Thunder Wave", "Recover"],
        },
        {
            "species": "Chansey",
            "moves": ["Ice Beam", "Thunderbolt", "Thunder Wave", "Soft-Boiled"],
        },
        {
            "species": "Snorlax",
            "moves": ["Body Slam", "Self-Destruct", "Reflect", "Rest"],
        },
        {
            "species": "Tauros",
            "moves": ["Body Slam", "Earthquake", "Blizzard", "Hyper Beam"],
        },
    ],
    [
        {
            "species": "Gengar",
            "moves": ["Psychic", "Thunderbolt", "Hypnosis", "Explosion"],
        },
        {
            "species": "Exeggutor",
            "moves": ["Psychic", "Stun Spore", "Mega Drain", "Explosion"],
        },
        {
            "species": "Starmie",
            "moves": ["Surf", "Thunderbolt", "Thunder Wave", "Recover"],
        },
        {
            "species": "Chansey",
            "moves": ["Sing", "Seismic Toss", "Thunder Wave", "Soft-Boiled"],
        },
        {"species": "Snorlax", "moves": ["Body Slam", "Hyper Beam", "Reflect", "Rest"]},
        {
            "species": "Tauros",
            "moves": ["Body Slam", "Earthquake", "Blizzard", "Hyper Beam"],
        },
    ],
    [
        {
            "species": "Alakazam",
            "moves": ["Psychic", "Seismic Toss", "Thunder Wave", "Recover"],
        },
        {
            "species": "Exeggutor",
            "moves": ["Sleep Powder", "Psychic", "Mega Drain", "Explosion"],
        },
        {
            "species": "Zapdos",
            "moves": ["Thunderbolt", "Drill Peck", "Thunder Wave", "Agility"],
        },
        {
            "species": "Chansey",
            "moves": ["Reflect", "Seismic Toss", "Thunder Wave", "Soft-Boiled"],
        },
        {
            "species": "Snorlax",
            "moves": ["Body Slam", "Earthquake", "Hyper Beam", "Self-Destruct"],
        },
        {
            "species": "Tauros",
            "moves": ["Body Slam", "Earthquake", "Blizzard", "Hyper Beam"],
        },
    ],
    [
        {"species": "Jynx", "moves": ["Lovely Kiss", "Psychic", "Blizzard", "Rest"]},
        {
            "species": "Starmie",
            "moves": ["Psychic", "Blizzard", "Thunder Wave", "Recover"],
        },
        {
            "species": "Rhydon",
            "moves": ["Earthquake", "Body Slam", "Substitute", "Rock Slide"],
        },
        {
            "species": "Chansey",
            "moves": ["Ice Beam", "Thunderbolt", "Thunder Wave", "Soft-Boiled"],
        },
        {
            "species": "Snorlax",
            "moves": ["Body Slam", "Self-Destruct", "Reflect", "Rest"],
        },
        {
            "species": "Tauros",
            "moves": ["Body Slam", "Earthquake", "Blizzard", "Hyper Beam"],
        },
    ],
    [
        {
            "species": "Starmie",
            "moves": ["Psychic", "Blizzard", "Thunder Wave", "Recover"],
        },
        {
            "species": "Exeggutor",
            "moves": ["Psychic", "Sleep Powder", "Stun Spore", "Mega Drain"],
        },
        {
            "species": "Rhydon",
            "moves": ["Earthquake", "Body Slam", "Substitute", "Rock Slide"],
        },
        {
            "species": "Chansey",
            "moves": ["Sing", "Seismic Toss", "Thunder Wave", "Soft-Boiled"],
        },
        {
            "species": "Snorlax",
            "moves": ["Body Slam", "Self-Destruct", "Reflect", "Rest"],
        },
        {
            "species": "Tauros",
            "moves": ["Body Slam", "Earthquake", "Blizzard", "Hyper Beam"],
        },
    ],
    [
        {
            "species": "Alakazam",
            "moves": ["Psychic", "Seismic Toss", "Thunder Wave", "Recover"],
        },
        {
            "species": "Exeggutor",
            "moves": ["Psychic", "Sleep Powder", "Mega Drain", "Explosion"],
        },
        {
            "species": "Gengar",
            "moves": ["Hypnosis", "Psychic", "Thunderbolt", "Explosion"],
        },
        {
            "species": "Chansey",
            "moves": ["Ice Beam", "Thunderbolt", "Thunder Wave", "Soft-Boiled"],
        },
        {
            "species": "Snorlax",
            "moves": ["Body Slam", "Earthquake", "Self-Destruct", "Rest"],
        },
        {
            "species": "Tauros",
            "moves": ["Body Slam", "Earthquake", "Blizzard", "Hyper Beam"],
        },
    ],
    [
        {
            "species": "Gengar",
            "moves": ["Night Shade", "Thunderbolt", "Hypnosis", "Explosion"],
        },
        {
            "species": "Exeggutor",
            "moves": ["Psychic", "Sleep Powder", "Stun Spore", "Explosion"],
        },
        {
            "species": "Starmie",
            "moves": ["Surf", "Thunderbolt", "Thunder Wave", "Recover"],
        },
        {
            "species": "Zapdos",
            "moves": ["Thunderbolt", "Drill Peck", "Thunder Wave", "Agility"],
        },
        {
            "species": "Snorlax",
            "moves": ["Body Slam", "Earthquake", "Counter", "Self-Destruct"],
        },
        {
            "species": "Tauros",
            "moves": ["Body Slam", "Earthquake", "Blizzard", "Hyper Beam"],
        },
    ],
    [
        {
            "species": "Starmie",
            "moves": ["Psychic", "Blizzard", "Thunder Wave", "Recover"],
        },
        {
            "species": "Exeggutor",
            "moves": ["Psychic", "Sleep Powder", "Stun Spore", "Explosion"],
        },
        {
            "species": "Zapdos",
            "moves": ["Thunderbolt", "Drill Peck", "Thunder Wave", "Agility"],
        },
        {
            "species": "Chansey",
            "moves": ["Ice Beam", "Thunderbolt", "Thunder Wave", "Soft-Boiled"],
        },
        {"species": "Snorlax", "moves": ["Body Slam", "Earthquake", "Reflect", "Rest"]},
        {
            "species": "Tauros",
            "moves": ["Body Slam", "Earthquake", "Blizzard", "Hyper Beam"],
        },
    ],
    [
        {
            "species": "Starmie",
            "moves": ["Psychic", "Blizzard", "Thunder Wave", "Recover"],
        },
        {
            "species": "Exeggutor",
            "moves": ["Psychic", "Sleep Powder", "Stun Spore", "Explosion"],
        },
        {"species": "Cloyster", "moves": ["Blizzard", "Clamp", "Rest", "Explosion"]},
        {
            "species": "Chansey",
            "moves": ["Sing", "Seismic Toss", "Thunder Wave", "Soft-Boiled"],
        },
        {
            "species": "Snorlax",
            "moves": ["Body Slam", "Earthquake", "Hyper Beam", "Self-Destruct"],
        },
        {
            "species": "Tauros",
            "moves": ["Body Slam", "Earthquake", "Blizzard", "Hyper Beam"],
        },
    ],
    [
        {"species": "Starmie", "moves": ["Psychic", "Thunder Wave", "Recover", "Surf"]},
        {
            "species": "Exeggutor",
            "moves": ["Psychic", "Sleep Powder", "Explosion", "Stun Spore"],
        },
        {
            "species": "Alakazam",
            "moves": ["Psychic", "Seismic Toss", "Recover", "Reflect"],
        },
        {
            "species": "Chansey",
            "moves": ["Seismic Toss", "Soft-Boiled", "Toxic", "Ice Beam"],
        },
        {
            "species": "Snorlax",
            "moves": ["Body Slam", "Rest", "Ice Beam", "Self-Destruct"],
        },
        {
            "species": "Tauros",
            "moves": ["Body Slam", "Hyper Beam", "Earthquake", "Blizzard"],
        },
    ],
    [
        {
            "species": "Gengar",
            "moves": ["Hypnosis", "Psychic", "Thunderbolt", "Mega Drain"],
        },
        {
            "species": "Starmie",
            "moves": ["Blizzard", "Surf", "Thunder Wave", "Recover"],
        },
        {
            "species": "Exeggutor",
            "moves": ["Psychic", "Sleep Powder", "Explosion", "Mega Drain"],
        },
        {
            "species": "Chansey",
            "moves": ["Ice Beam", "Thunderbolt", "Soft-Boiled", "Thunder Wave"],
        },
        {"species": "Snorlax", "moves": ["Amnesia", "Body Slam", "Rest", "Ice Beam"]},
        {
            "species": "Tauros",
            "moves": ["Body Slam", "Hyper Beam", "Earthquake", "Blizzard"],
        },
    ],
    [
        {"species": "Jynx", "moves": ["Lovely Kiss", "Psychic", "Blizzard", "Rest"]},
        {
            "species": "Exeggutor",
            "moves": ["Psychic", "Stun Spore", "Mega Drain", "Explosion"],
        },
        {
            "species": "Zapdos",
            "moves": ["Thunderbolt", "Drill Peck", "Agility", "Thunder Wave"],
        },
        {
            "species": "Rhydon",
            "moves": ["Earthquake", "Rock Slide", "Substitute", "Body Slam"],
        },
        {
            "species": "Snorlax",
            "moves": ["Body Slam", "Rest", "Earthquake", "Self-Destruct"],
        },
        {
            "species": "Tauros",
            "moves": ["Body Slam", "Hyper Beam", "Blizzard", "Earthquake"],
        },
    ],
    [
        {
            "species": "Starmie",
            "moves": ["Psychic", "Thunderbolt", "Blizzard", "Recover"],
        },
        {
            "species": "Alakazam",
            "moves": ["Psychic", "Seismic Toss", "Recover", "Thunder Wave"],
        },
        {
            "species": "Exeggutor",
            "moves": ["Sleep Powder", "Psychic", "Explosion", "Double-Edge"],
        },
        {
            "species": "Chansey",
            "moves": ["Soft-Boiled", "Seismic Toss", "Ice Beam", "Thunder Wave"],
        },
        {
            "species": "Snorlax",
            "moves": ["Body Slam", "Hyper Beam", "Self-Destruct", "Earthquake"],
        },
        {
            "species": "Tauros",
            "moves": ["Body Slam", "Earthquake", "Blizzard", "Fire Blast"],
        },
    ],
    [
        {
            "species": "Alakazam",
            "moves": ["Psychic", "Thunder Wave", "Reflect", "Recover"],
        },
        {
            "species": "Gengar",
            "moves": ["Hypnosis", "Thunderbolt", "Psychic", "Explosion"],
        },
        {
            "species": "Starmie",
            "moves": ["Blizzard", "Surf", "Recover", "Thunder Wave"],
        },
        {
            "species": "Chansey",
            "moves": ["Seismic Toss", "Soft-Boiled", "Thunder Wave", "Ice Beam"],
        },
        {"species": "Snorlax", "moves": ["Body Slam", "Rest", "Reflect", "Earthquake"]},
        {
            "species": "Tauros",
            "moves": ["Body Slam", "Hyper Beam", "Blizzard", "Earthquake"],
        },
    ],
    [
        {
            "species": "Jynx",
            "moves": ["Lovely Kiss", "Blizzard", "Body Slam", "Psychic"],
        },
        {
            "species": "Exeggutor",
            "moves": ["Psychic", "Sleep Powder", "Stun Spore", "Explosion"],
        },
        {
            "species": "Zapdos",
            "moves": ["Thunderbolt", "Drill Peck", "Thunder Wave", "Reflect"],
        },
        {
            "species": "Chansey",
            "moves": ["Seismic Toss", "Soft-Boiled", "Sing", "Thunder Wave"],
        },
        {
            "species": "Snorlax",
            "moves": ["Body Slam", "Earthquake", "Hyper Beam", "Rest"],
        },
        {
            "species": "Tauros",
            "moves": ["Body Slam", "Blizzard", "Earthquake", "Hyper Beam"],
        },
    ],
    [
        {
            "species": "Starmie",
            "moves": ["Blizzard", "Psychic", "Recover", "Thunder Wave"],
        },
        {
            "species": "Exeggutor",
            "moves": ["Psychic", "Sleep Powder", "Explosion", "Mega Drain"],
        },
        {
            "species": "Lapras",
            "moves": ["Blizzard", "Body Slam", "Rest", "Confuse Ray"],
        },
        {
            "species": "Chansey",
            "moves": ["Soft-Boiled", "Seismic Toss", "Thunder Wave", "Ice Beam"],
        },
        {
            "species": "Snorlax",
            "moves": ["Body Slam", "Earthquake", "Self-Destruct", "Reflect"],
        },
        {
            "species": "Tauros",
            "moves": ["Body Slam", "Hyper Beam", "Earthquake", "Blizzard"],
        },
    ],
    [
        {
            "species": "Gengar",
            "moves": ["Hypnosis", "Thunderbolt", "Night Shade", "Explosion"],
        },
        {
            "species": "Exeggutor",
            "moves": ["Psychic", "Stun Spore", "Sleep Powder", "Mega Drain"],
        },
        {"species": "Starmie", "moves": ["Psychic", "Blizzard", "Recover", "Surf"]},
        {
            "species": "Zapdos",
            "moves": ["Thunderbolt", "Drill Peck", "Thunder Wave", "Agility"],
        },
        {"species": "Snorlax", "moves": ["Body Slam", "Earthquake", "Rest", "Reflect"]},
        {
            "species": "Tauros",
            "moves": ["Body Slam", "Hyper Beam", "Blizzard", "Earthquake"],
        },
    ],
    [
        {
            "species": "Alakazam",
            "moves": ["Psychic", "Seismic Toss", "Thunder Wave", "Recover"],
        },
        {
            "species": "Exeggutor",
            "moves": ["Sleep Powder", "Psychic", "Stun Spore", "Explosion"],
        },
        {
            "species": "Starmie",
            "moves": ["Blizzard", "Thunderbolt", "Recover", "Thunder Wave"],
        },
        {
            "species": "Rhydon",
            "moves": ["Earthquake", "Rock Slide", "Body Slam", "Substitute"],
        },
        {
            "species": "Snorlax",
            "moves": ["Body Slam", "Self-Destruct", "Rest", "Ice Beam"],
        },
        {
            "species": "Tauros",
            "moves": ["Body Slam", "Hyper Beam", "Earthquake", "Blizzard"],
        },
    ],
    [
        {"species": "Jynx", "moves": ["Lovely Kiss", "Psychic", "Blizzard", "Counter"]},
        {
            "species": "Starmie",
            "moves": ["Psychic", "Blizzard", "Thunder Wave", "Recover"],
        },
        {
            "species": "Exeggutor",
            "moves": ["Psychic", "Sleep Powder", "Mega Drain", "Explosion"],
        },
        {
            "species": "Chansey",
            "moves": ["Seismic Toss", "Soft-Boiled", "Thunder Wave", "Reflect"],
        },
        {
            "species": "Snorlax",
            "moves": ["Body Slam", "Earthquake", "Hyper Beam", "Self-Destruct"],
        },
        {
            "species": "Tauros",
            "moves": ["Body Slam", "Blizzard", "Earthquake", "Hyper Beam"],
        },
    ],
    [
        {
            "species": "Gengar",
            "moves": ["Hypnosis", "Psychic", "Mega Drain", "Thunderbolt"],
        },
        {
            "species": "Alakazam",
            "moves": ["Psychic", "Seismic Toss", "Recover", "Reflect"],
        },
        {
            "species": "Exeggutor",
            "moves": ["Sleep Powder", "Psychic", "Explosion", "Stun Spore"],
        },
        {
            "species": "Chansey",
            "moves": ["Soft-Boiled", "Seismic Toss", "Ice Beam", "Thunder Wave"],
        },
        {
            "species": "Snorlax",
            "moves": ["Body Slam", "Rest", "Earthquake", "Hyper Beam"],
        },
        {
            "species": "Tauros",
            "moves": ["Body Slam", "Blizzard", "Earthquake", "Hyper Beam"],
        },
    ],
    [
        {
            "species": "Starmie",
            "moves": ["Surf", "Blizzard", "Recover", "Thunder Wave"],
        },
        {
            "species": "Exeggutor",
            "moves": ["Psychic", "Stun Spore", "Sleep Powder", "Explosion"],
        },
        {
            "species": "Zapdos",
            "moves": ["Thunderbolt", "Drill Peck", "Thunder Wave", "Agility"],
        },
        {
            "species": "Chansey",
            "moves": ["Seismic Toss", "Soft-Boiled", "Ice Beam", "Thunder Wave"],
        },
        {"species": "Snorlax", "moves": ["Amnesia", "Rest", "Body Slam", "Ice Beam"]},
        {
            "species": "Tauros",
            "moves": ["Body Slam", "Hyper Beam", "Earthquake", "Blizzard"],
        },
    ],
    [
        {
            "species": "Alakazam",
            "moves": ["Psychic", "Thunder Wave", "Seismic Toss", "Recover"],
        },
        {"species": "Jynx", "moves": ["Lovely Kiss", "Blizzard", "Psychic", "Rest"]},
        {
            "species": "Starmie",
            "moves": ["Blizzard", "Surf", "Thunder Wave", "Recover"],
        },
        {
            "species": "Chansey",
            "moves": ["Seismic Toss", "Soft-Boiled", "Sing", "Thunder Wave"],
        },
        {
            "species": "Snorlax",
            "moves": ["Body Slam", "Earthquake", "Rest", "Self-Destruct"],
        },
        {
            "species": "Tauros",
            "moves": ["Body Slam", "Hyper Beam", "Blizzard", "Earthquake"],
        },
    ],
    [
        {
            "species": "Gengar",
            "moves": ["Hypnosis", "Thunderbolt", "Psychic", "Explosion"],
        },
        {
            "species": "Exeggutor",
            "moves": ["Psychic", "Sleep Powder", "Explosion", "Mega Drain"],
        },
        {"species": "Cloyster", "moves": ["Blizzard", "Clamp", "Rest", "Explosion"]},
        {
            "species": "Chansey",
            "moves": ["Seismic Toss", "Soft-Boiled", "Thunder Wave", "Ice Beam"],
        },
        {
            "species": "Snorlax",
            "moves": ["Body Slam", "Hyper Beam", "Earthquake", "Self-Destruct"],
        },
        {
            "species": "Tauros",
            "moves": ["Body Slam", "Blizzard", "Earthquake", "Hyper Beam"],
        },
    ],
    [
        {
            "species": "Starmie",
            "moves": ["Psychic", "Blizzard", "Recover", "Thunder Wave"],
        },
        {
            "species": "Exeggutor",
            "moves": ["Sleep Powder", "Psychic", "Double-Edge", "Explosion"],
        },
        {
            "species": "Zapdos",
            "moves": ["Thunderbolt", "Drill Peck", "Agility", "Thunder Wave"],
        },
        {
            "species": "Rhydon",
            "moves": ["Earthquake", "Rock Slide", "Substitute", "Rest"],
        },
        {"species": "Snorlax", "moves": ["Body Slam", "Rest", "Reflect", "Earthquake"]},
        {
            "species": "Tauros",
            "moves": ["Body Slam", "Hyper Beam", "Blizzard", "Earthquake"],
        },
    ],
    [
        {
            "species": "Jynx",
            "moves": ["Lovely Kiss", "Blizzard", "Psychic", "Body Slam"],
        },
        {
            "species": "Alakazam",
            "moves": ["Psychic", "Seismic Toss", "Recover", "Thunder Wave"],
        },
        {
            "species": "Exeggutor",
            "moves": ["Psychic", "Stun Spore", "Explosion", "Sleep Powder"],
        },
        {
            "species": "Chansey",
            "moves": ["Soft-Boiled", "Seismic Toss", "Thunder Wave", "Ice Beam"],
        },
        {"species": "Snorlax", "moves": ["Amnesia", "Body Slam", "Rest", "Ice Beam"]},
        {
            "species": "Tauros",
            "moves": ["Body Slam", "Hyper Beam", "Earthquake", "Blizzard"],
        },
    ],
    [
        {
            "species": "Gengar",
            "moves": ["Hypnosis", "Night Shade", "Thunderbolt", "Explosion"],
        },
        {"species": "Starmie", "moves": ["Psychic", "Surf", "Blizzard", "Recover"]},
        {
            "species": "Exeggutor",
            "moves": ["Psychic", "Sleep Powder", "Stun Spore", "Mega Drain"],
        },
        {
            "species": "Chansey",
            "moves": ["Seismic Toss", "Soft-Boiled", "Thunder Wave", "Ice Beam"],
        },
        {
            "species": "Snorlax",
            "moves": ["Body Slam", "Earthquake", "Hyper Beam", "Rest"],
        },
        {
            "species": "Tauros",
            "moves": ["Body Slam", "Blizzard", "Earthquake", "Fire Blast"],
        },
    ],
    [
        {
            "species": "Alakazam",
            "moves": ["Psychic", "Seismic Toss", "Thunder Wave", "Reflect"],
        },
        {
            "species": "Exeggutor",
            "moves": ["Sleep Powder", "Psychic", "Explosion", "Double-Edge"],
        },
        {
            "species": "Starmie",
            "moves": ["Blizzard", "Thunderbolt", "Recover", "Thunder Wave"],
        },
        {
            "species": "Lapras",
            "moves": ["Blizzard", "Thunderbolt", "Body Slam", "Rest"],
        },
        {
            "species": "Snorlax",
            "moves": ["Body Slam", "Self-Destruct", "Earthquake", "Rest"],
        },
        {
            "species": "Tauros",
            "moves": ["Body Slam", "Hyper Beam", "Blizzard", "Earthquake"],
        },
    ],
    [
        {
            "species": "Starmie",
            "moves": ["Blizzard", "Psychic", "Thunder Wave", "Recover"],
        },
        {"species": "Jynx", "moves": ["Lovely Kiss", "Blizzard", "Psychic", "Rest"]},
        {
            "species": "Exeggutor",
            "moves": ["Psychic", "Stun Spore", "Mega Drain", "Explosion"],
        },
        {
            "species": "Chansey",
            "moves": ["Seismic Toss", "Soft-Boiled", "Ice Beam", "Thunder Wave"],
        },
        {"species": "Snorlax", "moves": ["Body Slam", "Reflect", "Rest", "Earthquake"]},
        {
            "species": "Tauros",
            "moves": ["Body Slam", "Hyper Beam", "Blizzard", "Earthquake"],
        },
    ],
    [
        {
            "species": "Gengar",
            "moves": ["Hypnosis", "Psychic", "Thunderbolt", "Mega Drain"],
        },
        {
            "species": "Alakazam",
            "moves": ["Psychic", "Thunder Wave", "Seismic Toss", "Recover"],
        },
        {
            "species": "Exeggutor",
            "moves": ["Sleep Powder", "Psychic", "Explosion", "Stun Spore"],
        },
        {
            "species": "Zapdos",
            "moves": ["Thunderbolt", "Drill Peck", "Thunder Wave", "Agility"],
        },
        {
            "species": "Snorlax",
            "moves": ["Body Slam", "Earthquake", "Rest", "Self-Destruct"],
        },
        {
            "species": "Tauros",
            "moves": ["Body Slam", "Hyper Beam", "Blizzard", "Earthquake"],
        },
    ],
    [
        {"species": "Jynx", "moves": ["Lovely Kiss", "Psychic", "Blizzard", "Counter"]},
        {
            "species": "Starmie",
            "moves": ["Surf", "Blizzard", "Recover", "Thunder Wave"],
        },
        {
            "species": "Exeggutor",
            "moves": ["Psychic", "Sleep Powder", "Stun Spore", "Explosion"],
        },
        {
            "species": "Rhydon",
            "moves": ["Earthquake", "Rock Slide", "Body Slam", "Substitute"],
        },
        {
            "species": "Snorlax",
            "moves": ["Body Slam", "Hyper Beam", "Self-Destruct", "Earthquake"],
        },
        {
            "species": "Tauros",
            "moves": ["Body Slam", "Earthquake", "Blizzard", "Fire Blast"],
        },
    ],
    [
        {
            "species": "Alakazam",
            "moves": ["Psychic", "Seismic Toss", "Recover", "Thunder Wave"],
        },
        {
            "species": "Gengar",
            "moves": ["Hypnosis", "Psychic", "Thunderbolt", "Explosion"],
        },
        {
            "species": "Exeggutor",
            "moves": ["Psychic", "Stun Spore", "Sleep Powder", "Mega Drain"],
        },
        {
            "species": "Chansey",
            "moves": ["Seismic Toss", "Soft-Boiled", "Thunder Wave", "Reflect"],
        },
        {"species": "Snorlax", "moves": ["Body Slam", "Rest", "Reflect", "Earthquake"]},
        {
            "species": "Tauros",
            "moves": ["Body Slam", "Hyper Beam", "Earthquake", "Blizzard"],
        },
    ],
    [
        {"species": "Starmie", "moves": ["Psychic", "Blizzard", "Surf", "Recover"]},
        {
            "species": "Exeggutor",
            "moves": ["Sleep Powder", "Psychic", "Explosion", "Mega Drain"],
        },
        {
            "species": "Zapdos",
            "moves": ["Thunderbolt", "Drill Peck", "Thunder Wave", "Rest"],
        },
        {
            "species": "Chansey",
            "moves": ["Soft-Boiled", "Seismic Toss", "Ice Beam", "Thunder Wave"],
        },
        {
            "species": "Snorlax",
            "moves": ["Body Slam", "Earthquake", "Self-Destruct", "Hyper Beam"],
        },
        {
            "species": "Tauros",
            "moves": ["Body Slam", "Blizzard", "Earthquake", "Hyper Beam"],
        },
    ],
    [
        {
            "species": "Jynx",
            "moves": ["Lovely Kiss", "Blizzard", "Body Slam", "Psychic"],
        },
        {
            "species": "Exeggutor",
            "moves": ["Psychic", "Stun Spore", "Explosion", "Sleep Powder"],
        },
        {
            "species": "Starmie",
            "moves": ["Blizzard", "Psychic", "Thunder Wave", "Recover"],
        },
        {"species": "Cloyster", "moves": ["Blizzard", "Explosion", "Clamp", "Rest"]},
        {
            "species": "Snorlax",
            "moves": ["Body Slam", "Earthquake", "Rest", "Self-Destruct"],
        },
        {
            "species": "Tauros",
            "moves": ["Body Slam", "Hyper Beam", "Blizzard", "Earthquake"],
        },
    ],
    [
        {
            "species": "Gengar",
            "moves": ["Hypnosis", "Thunderbolt", "Night Shade", "Explosion"],
        },
        {
            "species": "Starmie",
            "moves": ["Blizzard", "Surf", "Thunder Wave", "Recover"],
        },
        {
            "species": "Exeggutor",
            "moves": ["Psychic", "Sleep Powder", "Mega Drain", "Explosion"],
        },
        {
            "species": "Zapdos",
            "moves": ["Thunderbolt", "Drill Peck", "Agility", "Thunder Wave"],
        },
        {"species": "Snorlax", "moves": ["Amnesia", "Body Slam", "Rest", "Ice Beam"]},
        {
            "species": "Tauros",
            "moves": ["Body Slam", "Hyper Beam", "Earthquake", "Blizzard"],
        },
    ],
    [
        {
            "species": "Alakazam",
            "moves": ["Psychic", "Thunder Wave", "Seismic Toss", "Recover"],
        },
        {
            "species": "Exeggutor",
            "moves": ["Sleep Powder", "Psychic", "Stun Spore", "Double-Edge"],
        },
        {
            "species": "Starmie",
            "moves": ["Blizzard", "Psychic", "Thunder Wave", "Recover"],
        },
        {
            "species": "Jolteon",
            "moves": ["Thunderbolt", "Thunder Wave", "Double Kick", "Pin Missile"],
        },
        {
            "species": "Snorlax",
            "moves": ["Body Slam", "Rest", "Hyper Beam", "Earthquake"],
        },
        {
            "species": "Tauros",
            "moves": ["Body Slam", "Blizzard", "Hyper Beam", "Earthquake"],
        },
    ],
    [
        {"species": "Starmie", "moves": ["Surf", "Psychic", "Blizzard", "Recover"]},
        {"species": "Jynx", "moves": ["Lovely Kiss", "Psychic", "Blizzard", "Rest"]},
        {
            "species": "Exeggutor",
            "moves": ["Psychic", "Stun Spore", "Sleep Powder", "Explosion"],
        },
        {
            "species": "Chansey",
            "moves": ["Seismic Toss", "Soft-Boiled", "Thunder Wave", "Ice Beam"],
        },
        {
            "species": "Snorlax",
            "moves": ["Body Slam", "Hyper Beam", "Earthquake", "Self-Destruct"],
        },
        {
            "species": "Tauros",
            "moves": ["Body Slam", "Earthquake", "Blizzard", "Fire Blast"],
        },
    ],
    [
        {
            "species": "Gengar",
            "moves": ["Hypnosis", "Psychic", "Thunderbolt", "Explosion"],
        },
        {
            "species": "Alakazam",
            "moves": ["Psychic", "Seismic Toss", "Thunder Wave", "Recover"],
        },
        {
            "species": "Exeggutor",
            "moves": ["Sleep Powder", "Psychic", "Explosion", "Stun Spore"],
        },
        {
            "species": "Zapdos",
            "moves": ["Thunderbolt", "Drill Peck", "Thunder Wave", "Rest"],
        },
        {"species": "Snorlax", "moves": ["Body Slam", "Earthquake", "Reflect", "Rest"]},
        {
            "species": "Tauros",
            "moves": ["Body Slam", "Hyper Beam", "Blizzard", "Earthquake"],
        },
    ],
    [
        {
            "species": "Jynx",
            "moves": ["Lovely Kiss", "Blizzard", "Psychic", "Body Slam"],
        },
        {
            "species": "Starmie",
            "moves": ["Blizzard", "Surf", "Thunder Wave", "Recover"],
        },
        {
            "species": "Rhydon",
            "moves": ["Earthquake", "Rock Slide", "Body Slam", "Rest"],
        },
        {
            "species": "Chansey",
            "moves": ["Seismic Toss", "Soft-Boiled", "Thunder Wave", "Sing"],
        },
        {
            "species": "Snorlax",
            "moves": ["Body Slam", "Rest", "Self-Destruct", "Earthquake"],
        },
        {
            "species": "Tauros",
            "moves": ["Body Slam", "Hyper Beam", "Blizzard", "Earthquake"],
        },
    ],
    [
        {
            "species": "Alakazam",
            "moves": ["Psychic", "Seismic Toss", "Recover", "Reflect"],
        },
        {
            "species": "Starmie",
            "moves": ["Psychic", "Blizzard", "Thunder Wave", "Recover"],
        },
        {
            "species": "Exeggutor",
            "moves": ["Sleep Powder", "Psychic", "Double-Edge", "Explosion"],
        },
        {
            "species": "Lapras",
            "moves": ["Blizzard", "Body Slam", "Thunderbolt", "Rest"],
        },
        {
            "species": "Snorlax",
            "moves": ["Body Slam", "Earthquake", "Hyper Beam", "Self-Destruct"],
        },
        {
            "species": "Tauros",
            "moves": ["Body Slam", "Blizzard", "Earthquake", "Hyper Beam"],
        },
    ],
    [
        {
            "species": "Gengar",
            "moves": ["Hypnosis", "Thunderbolt", "Psychic", "Mega Drain"],
        },
        {
            "species": "Exeggutor",
            "moves": ["Psychic", "Stun Spore", "Sleep Powder", "Mega Drain"],
        },
        {"species": "Starmie", "moves": ["Blizzard", "Psychic", "Surf", "Recover"]},
        {
            "species": "Chansey",
            "moves": ["Seismic Toss", "Soft-Boiled", "Ice Beam", "Thunder Wave"],
        },
        {"species": "Snorlax", "moves": ["Amnesia", "Rest", "Body Slam", "Ice Beam"]},
        {
            "species": "Tauros",
            "moves": ["Body Slam", "Hyper Beam", "Earthquake", "Blizzard"],
        },
    ],
    [
        {
            "species": "Starmie",
            "moves": ["Blizzard", "Thunderbolt", "Recover", "Thunder Wave"],
        },
        {"species": "Jynx", "moves": ["Lovely Kiss", "Blizzard", "Psychic", "Counter"]},
        {
            "species": "Exeggutor",
            "moves": ["Psychic", "Sleep Powder", "Explosion", "Mega Drain"],
        },
        {
            "species": "Zapdos",
            "moves": ["Thunderbolt", "Drill Peck", "Agility", "Thunder Wave"],
        },
        {
            "species": "Snorlax",
            "moves": ["Body Slam", "Earthquake", "Self-Destruct", "Rest"],
        },
        {
            "species": "Tauros",
            "moves": ["Body Slam", "Hyper Beam", "Blizzard", "Earthquake"],
        },
    ],
    [
        {
            "species": "Alakazam",
            "moves": ["Psychic", "Thunder Wave", "Seismic Toss", "Recover"],
        },
        {
            "species": "Gengar",
            "moves": ["Hypnosis", "Thunderbolt", "Night Shade", "Explosion"],
        },
        {
            "species": "Exeggutor",
            "moves": ["Psychic", "Stun Spore", "Explosion", "Sleep Powder"],
        },
        {
            "species": "Chansey",
            "moves": ["Seismic Toss", "Soft-Boiled", "Thunder Wave", "Ice Beam"],
        },
        {
            "species": "Snorlax",
            "moves": ["Body Slam", "Rest", "Earthquake", "Hyper Beam"],
        },
        {
            "species": "Tauros",
            "moves": ["Body Slam", "Blizzard", "Earthquake", "Fire Blast"],
        },
    ],
    [
        {"species": "Jynx", "moves": ["Lovely Kiss", "Psychic", "Blizzard", "Rest"]},
        {
            "species": "Starmie",
            "moves": ["Psychic", "Blizzard", "Thunder Wave", "Recover"],
        },
        {
            "species": "Exeggutor",
            "moves": ["Sleep Powder", "Psychic", "Stun Spore", "Double-Edge"],
        },
        {"species": "Cloyster", "moves": ["Blizzard", "Explosion", "Rest", "Surf"]},
        {
            "species": "Snorlax",
            "moves": ["Body Slam", "Earthquake", "Hyper Beam", "Self-Destruct"],
        },
        {
            "species": "Tauros",
            "moves": ["Body Slam", "Blizzard", "Earthquake", "Hyper Beam"],
        },
    ],
    [
        {
            "species": "Gengar",
            "moves": ["Hypnosis", "Psychic", "Thunderbolt", "Explosion"],
        },
        {"species": "Starmie", "moves": ["Blizzard", "Surf", "Psychic", "Recover"]},
        {
            "species": "Exeggutor",
            "moves": ["Psychic", "Sleep Powder", "Mega Drain", "Explosion"],
        },
        {
            "species": "Rhydon",
            "moves": ["Earthquake", "Rock Slide", "Substitute", "Body Slam"],
        },
        {"species": "Snorlax", "moves": ["Body Slam", "Rest", "Reflect", "Earthquake"]},
        {
            "species": "Tauros",
            "moves": ["Body Slam", "Hyper Beam", "Blizzard", "Earthquake"],
        },
    ],
    [
        {
            "species": "Alakazam",
            "moves": ["Psychic", "Seismic Toss", "Thunder Wave", "Recover"],
        },
        {
            "species": "Jynx",
            "moves": ["Lovely Kiss", "Blizzard", "Body Slam", "Psychic"],
        },
        {
            "species": "Exeggutor",
            "moves": ["Psychic", "Stun Spore", "Explosion", "Sleep Powder"],
        },
        {
            "species": "Zapdos",
            "moves": ["Thunderbolt", "Drill Peck", "Thunder Wave", "Agility"],
        },
        {"species": "Snorlax", "moves": ["Amnesia", "Body Slam", "Rest", "Ice Beam"]},
        {
            "species": "Tauros",
            "moves": ["Body Slam", "Hyper Beam", "Earthquake", "Blizzard"],
        },
    ],
    [
        {"species": "Starmie", "moves": ["Surf", "Blizzard", "Psychic", "Recover"]},
        {
            "species": "Exeggutor",
            "moves": ["Sleep Powder", "Psychic", "Explosion", "Double-Edge"],
        },
        {
            "species": "Zapdos",
            "moves": ["Thunderbolt", "Drill Peck", "Thunder Wave", "Rest"],
        },
        {
            "species": "Chansey",
            "moves": ["Seismic Toss", "Soft-Boiled", "Thunder Wave", "Reflect"],
        },
        {
            "species": "Snorlax",
            "moves": ["Body Slam", "Earthquake", "Self-Destruct", "Hyper Beam"],
        },
        {
            "species": "Tauros",
            "moves": ["Body Slam", "Blizzard", "Earthquake", "Hyper Beam"],
        },
    ],
    [
        {
            "species": "Gengar",
            "moves": ["Hypnosis", "Thunderbolt", "Psychic", "Mega Drain"],
        },
        {
            "species": "Alakazam",
            "moves": ["Psychic", "Seismic Toss", "Recover", "Thunder Wave"],
        },
        {
            "species": "Starmie",
            "moves": ["Blizzard", "Surf", "Thunder Wave", "Recover"],
        },
        {
            "species": "Chansey",
            "moves": ["Seismic Toss", "Soft-Boiled", "Ice Beam", "Thunder Wave"],
        },
        {"species": "Snorlax", "moves": ["Body Slam", "Rest", "Earthquake", "Reflect"]},
        {
            "species": "Tauros",
            "moves": ["Body Slam", "Hyper Beam", "Blizzard", "Earthquake"],
        },
    ],
    [
        {"species": "Jynx", "moves": ["Lovely Kiss", "Blizzard", "Psychic", "Counter"]},
        {
            "species": "Exeggutor",
            "moves": ["Psychic", "Sleep Powder", "Stun Spore", "Explosion"],
        },
        {
            "species": "Starmie",
            "moves": ["Blizzard", "Psychic", "Recover", "Thunder Wave"],
        },
        {
            "species": "Rhydon",
            "moves": ["Earthquake", "Rock Slide", "Body Slam", "Substitute"],
        },
        {
            "species": "Snorlax",
            "moves": ["Body Slam", "Hyper Beam", "Earthquake", "Self-Destruct"],
        },
        {
            "species": "Tauros",
            "moves": ["Body Slam", "Earthquake", "Blizzard", "Fire Blast"],
        },
    ],
    [
        {
            "species": "Alakazam",
            "moves": ["Psychic", "Thunder Wave", "Seismic Toss", "Reflect"],
        },
        {
            "species": "Gengar",
            "moves": ["Hypnosis", "Psychic", "Thunderbolt", "Explosion"],
        },
        {
            "species": "Exeggutor",
            "moves": ["Sleep Powder", "Psychic", "Explosion", "Mega Drain"],
        },
        {
            "species": "Lapras",
            "moves": ["Blizzard", "Thunderbolt", "Body Slam", "Rest"],
        },
        {
            "species": "Snorlax",
            "moves": ["Body Slam", "Earthquake", "Rest", "Self-Destruct"],
        },
        {
            "species": "Tauros",
            "moves": ["Body Slam", "Hyper Beam", "Blizzard", "Earthquake"],
        },
    ],
    [
        {"species": "Starmie", "moves": ["Blizzard", "Psychic", "Surf", "Recover"]},
        {"species": "Jynx", "moves": ["Lovely Kiss", "Psychic", "Blizzard", "Rest"]},
        {
            "species": "Exeggutor",
            "moves": ["Psychic", "Stun Spore", "Sleep Powder", "Mega Drain"],
        },
        {
            "species": "Zapdos",
            "moves": ["Thunderbolt", "Drill Peck", "Agility", "Thunder Wave"],
        },
        {"species": "Snorlax", "moves": ["Body Slam", "Reflect", "Rest", "Earthquake"]},
        {
            "species": "Tauros",
            "moves": ["Body Slam", "Hyper Beam", "Earthquake", "Blizzard"],
        },
    ],
    [
        {
            "species": "Gengar",
            "moves": ["Hypnosis", "Thunderbolt", "Night Shade", "Explosion"],
        },
        {
            "species": "Exeggutor",
            "moves": ["Psychic", "Sleep Powder", "Explosion", "Stun Spore"],
        },
        {
            "species": "Starmie",
            "moves": ["Blizzard", "Surf", "Thunder Wave", "Recover"],
        },
        {
            "species": "Chansey",
            "moves": ["Seismic Toss", "Soft-Boiled", "Thunder Wave", "Ice Beam"],
        },
        {"species": "Snorlax", "moves": ["Amnesia", "Body Slam", "Rest", "Ice Beam"]},
        {
            "species": "Tauros",
            "moves": ["Body Slam", "Hyper Beam", "Blizzard", "Earthquake"],
        },
    ],
    [
        {
            "species": "Alakazam",
            "moves": ["Psychic", "Seismic Toss", "Recover", "Thunder Wave"],
        },
        {
            "species": "Starmie",
            "moves": ["Psychic", "Blizzard", "Thunder Wave", "Recover"],
        },
        {
            "species": "Exeggutor",
            "moves": ["Sleep Powder", "Psychic", "Double-Edge", "Stun Spore"],
        },
        {
            "species": "Jolteon",
            "moves": ["Thunderbolt", "Thunder Wave", "Double Kick", "Pin Missile"],
        },
        {
            "species": "Snorlax",
            "moves": ["Body Slam", "Hyper Beam", "Earthquake", "Rest"],
        },
        {
            "species": "Tauros",
            "moves": ["Body Slam", "Blizzard", "Earthquake", "Hyper Beam"],
        },
    ],
    [
        {
            "species": "Jynx",
            "moves": ["Lovely Kiss", "Blizzard", "Body Slam", "Psychic"],
        },
        {
            "species": "Exeggutor",
            "moves": ["Psychic", "Sleep Powder", "Explosion", "Mega Drain"],
        },
        {"species": "Starmie", "moves": ["Blizzard", "Surf", "Psychic", "Recover"]},
        {"species": "Cloyster", "moves": ["Blizzard", "Clamp", "Explosion", "Rest"]},
        {
            "species": "Snorlax",
            "moves": ["Body Slam", "Earthquake", "Self-Destruct", "Rest"],
        },
        {
            "species": "Tauros",
            "moves": ["Body Slam", "Hyper Beam", "Blizzard", "Earthquake"],
        },
    ],
    [
        {
            "species": "Gengar",
            "moves": ["Hypnosis", "Psychic", "Thunderbolt", "Explosion"],
        },
        {
            "species": "Alakazam",
            "moves": ["Psychic", "Thunder Wave", "Seismic Toss", "Recover"],
        },
        {
            "species": "Exeggutor",
            "moves": ["Sleep Powder", "Psychic", "Stun Spore", "Explosion"],
        },
        {
            "species": "Rhydon",
            "moves": ["Earthquake", "Rock Slide", "Rest", "Body Slam"],
        },
        {"species": "Snorlax", "moves": ["Body Slam", "Rest", "Reflect", "Earthquake"]},
        {
            "species": "Tauros",
            "moves": ["Body Slam", "Hyper Beam", "Earthquake", "Blizzard"],
        },
    ],
    [
        {
            "species": "Starmie",
            "moves": ["Surf", "Blizzard", "Thunder Wave", "Recover"],
        },
        {"species": "Jynx", "moves": ["Lovely Kiss", "Psychic", "Blizzard", "Counter"]},
        {
            "species": "Exeggutor",
            "moves": ["Psychic", "Stun Spore", "Explosion", "Sleep Powder"],
        },
        {
            "species": "Zapdos",
            "moves": ["Thunderbolt", "Drill Peck", "Thunder Wave", "Rest"],
        },
        {
            "species": "Snorlax",
            "moves": ["Body Slam", "Earthquake", "Hyper Beam", "Self-Destruct"],
        },
        {
            "species": "Tauros",
            "moves": ["Body Slam", "Blizzard", "Earthquake", "Fire Blast"],
        },
    ],
    [
        {
            "species": "Alakazam",
            "moves": ["Psychic", "Seismic Toss", "Thunder Wave", "Reflect"],
        },
        {
            "species": "Exeggutor",
            "moves": ["Sleep Powder", "Psychic", "Explosion", "Double-Edge"],
        },
        {"species": "Starmie", "moves": ["Blizzard", "Psychic", "Surf", "Recover"]},
        {
            "species": "Chansey",
            "moves": ["Seismic Toss", "Soft-Boiled", "Thunder Wave", "Sing"],
        },
        {"species": "Snorlax", "moves": ["Amnesia", "Rest", "Body Slam", "Ice Beam"]},
        {
            "species": "Tauros",
            "moves": ["Body Slam", "Hyper Beam", "Blizzard", "Earthquake"],
        },
    ],
]


def get_random_ou_team():
    """Get a random OU team from the predefined list."""
    import random

    return random.choice(OU_TEAMS)


def get_ou_team(index):
    """Get a specific OU team by index."""
    if 0 <= index < len(OU_TEAMS):
        return OU_TEAMS[index]
    return get_random_ou_team()
