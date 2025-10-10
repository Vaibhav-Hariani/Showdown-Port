# Auto-generated from move_labels.h
# Provides MOVE_LABELS list for Python-side decoding of move ids.
MOVE_LABELS = [
    "nomove", "pound", "karatechop", "doubleslap",
    "cometpunch", "megapunch", "payday", "firepunch",
    "icepunch", "thunderpunch", "scratch", "vicegrip",
    "guillotine", "razorwind", "swordsdance", "cut",
    "gust", "wingattack", "whirlwind", "fly",
    "bind", "slam", "vinewhip", "stomp",
    "doublekick", "megakick", "jumpkick", "rollingkick",
    "sandattack", "headbutt", "hornattack", "furyattack",
    "horndrill", "tackle", "bodyslam", "wrap",
    "takedown", "thrash", "doubleedge", "tailwhip",
    "poisonsting", "twineedle", "pinmissile", "leer",
    "bite", "growl", "roar", "sing",
    "supersonic", "sonicboom", "disable", "acid",
    "ember", "flamethrower", "mist", "watergun",
    "hydropump", "surf", "icebeam", "blizzard",
    "psybeam", "bubblebeam", "aurorabeam", "hyperbeam",
    "peck", "drillpeck", "submission", "lowkick",
    "counter", "seismictoss", "strength", "absorb",
    "megadrain", "leechseed", "growth", "razorleaf",
    "solarbeam", "poisonpowder", "stunspore", "sleeppowder",
    "petaldance", "stringshot", "dragonrage", "firespin",
    "thundershock", "thunderbolt", "thunderwave", "thunder",
    "rockthrow", "earthquake", "fissure", "dig",
    "toxic", "confusion", "psychic", "hypnosis",
    "meditate", "agility", "quickattack", "rage",
    "teleport", "nightshade", "mimic", "screech",
    "doubleteam", "recover", "harden", "minimize",
    "smokescreen", "confuseray", "withdraw", "defensecurl",
    "barrier", "lightscreen", "haze", "reflect",
    "focusenergy", "bide", "metronome", "mirrormove",
    "selfdestruct", "eggbomb", "lick", "smog",
    "sludge", "boneclub", "fireblast", "waterfall",
    "clamp", "swift", "skullbash", "spikecannon",
    "constrict", "amnesia", "kinesis", "softboiled",
    "highjumpkick", "glare", "dreameater", "poisongas",
    "barrage", "leechlife", "lovelykiss", "skyattack",
    "transform", "bubble", "dizzypunch", "spore",
    "flash", "psywave", "splash", "acidarmor",
    "crabhammer", "explosion", "furyswipes", "bonemerang",
    "rest", "rockslide", "hyperfang", "sharpen",
    "conversion", "triattack", "superfang", "slash",
    "substitute", "struggle"
]

# Safe accessor to avoid IndexErrors
def move_name(move_id: int) -> str:
    if 0 <= move_id < len(MOVE_LABELS):
        return MOVE_LABELS[move_id]
    return f"move_{move_id}"  # fallback for out-of-range
