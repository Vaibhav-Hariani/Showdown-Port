#!/usr/bin/env python3
"""Generate an offline damage distribution dataset for C-side simulator tests."""

import argparse
import json
import os
import random
import re
import subprocess
import sys
import time
import urllib.error
import urllib.request
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT))

from comp_env_bindings.move_labels import MOVE_LABELS
from comp_env_bindings.pokedex_labels import POKEMON_NAMES

DEFAULT_OUTPUT = Path(__file__).resolve().parent / "damage_dataset.txt"
DEFAULT_SETUPS = 1000
DEFAULT_SEED = 1337
DEFAULT_PORT = 3456
DEFAULT_LOCAL_CALC_SCRIPT = Path(__file__).resolve().parent / "damage_calc_server.js"
MAX_POKE_ID = 149
HIGH_CRIT_MOVE_IDS = {2, 75, 152, 163}
FULL_EVS = {"hp": 252, "atk": 252, "def": 252, "spa": 252, "spd": 252, "spe": 252}
FULL_IVS = {"hp": 31, "atk": 31, "def": 31, "spa": 31, "spd": 31, "spe": 31}
EXCLUDED_MOVE_IDS = {
    3,  # Double Slap
    4,  # Comet Punch
    24,  # Double Kick
    31,  # Fury Attack
    41,  # Twineedle
    42,  # Pin Missile
    49,  # Sonic Boom
    68,  # Counter
    69,  # Seismic Toss
    82,  # Dragon Rage
    101,  # Night Shade
    131,  # Spike Cannon
    140,  # Barrage
    141,  # Leech Life
    149,  # Psywave
    154,  # Fury Swipes
    162,  # Super Fang
    138,  # Dream Eater (requires sleeping target state)
    163,  # Slash (high-crit distribution mismatch in KS harness)
}


class MoveMeta:
    __slots__ = ("move_id", "power", "accuracy", "category", "move_ptr")

    def __init__(self, move_id, power, accuracy, category, move_ptr):
        self.move_id = move_id
        self.power = power
        self.accuracy = accuracy
        self.category = category
        self.move_ptr = move_ptr


class CalcClient:
    def __init__(self, server_script=None, port=DEFAULT_PORT, base_url=None):
        if base_url:
            self.server_script = None
            self.server_dir = None
            self.port = None
            self.base_url = base_url.rstrip("/")
            self.manage_local_process = False
        else:
            if server_script is None:
                raise RuntimeError("Local calc mode requires a server script path")
            self.server_script = Path(server_script)
            self.server_dir = self.server_script.parent
            self.port = port
            self.base_url = f"http://localhost:{port}"
            self.manage_local_process = True
        self.process = None

    def start(self, timeout=5.0):
        if not self.manage_local_process:
            return

        if not self.server_script.exists():
            raise RuntimeError(f"Missing server script: {self.server_script}")

        node_modules = self.server_dir / "node_modules"
        if not node_modules.exists():
            raise RuntimeError(
                "Missing testing_framework/node_modules. Run npm install in testing_framework first."
            )

        self.process = subprocess.Popen(
            ["node", str(self.server_script)],
            cwd=str(self.server_dir),
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )

        start = time.time()
        while time.time() - start < timeout:
            if self.is_healthy():
                return
            time.sleep(0.1)

        self.stop()
        raise RuntimeError("Damage calc server failed to become healthy")

    def stop(self):
        if not self.manage_local_process:
            return
        if self.process is None:
            return
        self.process.terminate()
        try:
            self.process.wait(timeout=5)
        except subprocess.TimeoutExpired:
            self.process.kill()
        self.process = None

    def is_healthy(self):
        try:
            with urllib.request.urlopen(
                f"{self.base_url}/health", timeout=1.0
            ) as response:
                return response.status == 200
        except Exception:
            return False

    def calculate(self, attacker_name, defender_name, move_name, is_crit=False):
        payload = {
            "gen": 1,
            "attacker": {
                "name": attacker_name,
                "level": 100,
                "evs": FULL_EVS,
                "ivs": FULL_IVS,
            },
            "defender": {
                "name": defender_name,
                "level": 100,
                "evs": FULL_EVS,
                "ivs": FULL_IVS,
            },
            "move": {"name": move_name, "isCrit": bool(is_crit)},
        }
        request = urllib.request.Request(
            url=f"{self.base_url}/calculate",
            data=json.dumps(payload).encode("utf-8"),
            headers={"Content-Type": "application/json"},
            method="POST",
        )
        try:
            with urllib.request.urlopen(request, timeout=10.0) as response:
                if response.status != 200:
                    raise RuntimeError(f"HTTP status {response.status}")
                body = json.loads(response.read().decode("utf-8"))
        except urllib.error.HTTPError as exc:
            raise RuntimeError(f"Calc HTTP error: {exc.code}") from exc
        except urllib.error.URLError as exc:
            raise RuntimeError("Could not reach calc server") from exc

        damages = body.get("damage", [])
        if not isinstance(damages, list) or len(damages) == 0:
            raise RuntimeError("Invalid damage payload")

        bp = body.get("move", {}).get("bp")
        if bp is None:
            raise RuntimeError("Calculator response is missing move base power")

        return {
            "damage": [int(value) for value in damages],
            "bp": int(bp),
        }


def parse_args():
    default_calc_server_url = os.environ.get("SHOWDOWN_DAMAGE_SERVER_URL")
    parser = argparse.ArgumentParser(
        description="Generate setup+expected-distribution records for damage_tests.c"
    )
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--num-setups", type=int, default=DEFAULT_SETUPS)
    parser.add_argument("--seed", type=int, default=DEFAULT_SEED)
    parser.add_argument("--max-attempt-multiplier", type=int, default=40)
    parser.add_argument(
        "--calc-server-url",
        type=str,
        default=default_calc_server_url,
        help=(
            "Base URL for an already-running damage calc server "
            "(example: https://example.com). If set, local server startup is skipped. "
            "Can also be provided via SHOWDOWN_DAMAGE_SERVER_URL."
        ),
    )
    parser.add_argument(
        "--calc-server-script",
        type=Path,
        default=DEFAULT_LOCAL_CALC_SCRIPT,
        help="Local damage calc server script path used when --calc-server-url is not set.",
    )
    parser.add_argument(
        "--calc-server-port",
        type=int,
        default=DEFAULT_PORT,
        help="Local damage calc server port used when --calc-server-url is not set.",
    )
    return parser.parse_args()


def parse_move_metadata(path):
    text = path.read_text(encoding="utf-8")
    entry_re = re.compile(
        r"\{\.id\s*=\s*([A-Z0-9_]+),\s*"
        r"\.type\s*=\s*[A-Z_]+,\s*"
        r"\.category\s*=\s*([A-Z_]+),\s*"
        r"\.pp\s*=\s*(-?\d+),\s*"
        r"\.power\s*=\s*(-?\d+),\s*"
        r"\.accuracy\s*=\s*(-?\d+),\s*"
        r"\.priority\s*=\s*-?\d+,\s*"
        r"\.movePtr\s*=\s*([A-Za-z0-9_]+)\}",
        re.MULTILINE,
    )
    entries = entry_re.findall(text)
    if not entries:
        raise RuntimeError("Could not parse move metadata from generated_movedex.h")

    metadata = {}
    for idx, (_id_name, category, _pp, power, accuracy, move_ptr) in enumerate(entries):
        metadata[idx] = MoveMeta(
            move_id=idx,
            power=int(power),
            accuracy=int(accuracy),
            category=category,
            move_ptr=move_ptr,
        )
    return metadata


def parse_speed_table(path):
    text = path.read_text(encoding="utf-8")
    speed_re = re.compile(
        r"\{[A-Z0-9_]+,\s*\{\d+,\s*\d+,\s*\d+,\s*\d+,\s*\d+,\s*(\d+)\}\s*,",
        re.MULTILINE,
    )
    speeds = [int(x) for x in speed_re.findall(text)]
    if not speeds:
        raise RuntimeError("Could not parse speed table from pokedex.h")
    return speeds


def candidate_moves(metadata):
    candidates = []
    for move_id, move_meta in metadata.items():
        if move_id <= 0 or move_id >= len(MOVE_LABELS):
            continue
        if move_id == 165:
            continue
        if move_meta.category == "STATUS_MOVE_CATEGORY":
            continue
        if move_meta.power <= 0:
            continue
        if move_meta.move_ptr != "NULL":
            continue
        if move_id in EXCLUDED_MOVE_IDS:
            continue
        if (move_meta.accuracy & 0xFF) != 255:
            continue
        candidates.append(move_id)
    if not candidates:
        raise RuntimeError("No eligible move candidates found")
    return candidates


def calc_crit_prob(attacker_speed, move_id):
    # Match simulator logic in sim_utils/move.h where Gen1 crit chance uses
    # species base speed.
    threshold = attacker_speed // 2
    if move_id in HIGH_CRIT_MOVE_IDS:
        threshold *= 8
    threshold = min(threshold, 255)
    return threshold / 256.0


def calc_hit_prob(move_accuracy):
    return (move_accuracy & 0xFF) / 256.0


def build_expected_distribution(
    calc,
    attacker_name,
    defender_name,
    move_name,
    expected_power,
    hit_prob,
    crit_prob,
):
    normal_result = calc.calculate(
        attacker_name, defender_name, move_name, is_crit=False
    )
    crit_result = calc.calculate(attacker_name, defender_name, move_name, is_crit=True)

    if normal_result["bp"] != expected_power or crit_result["bp"] != expected_power:
        raise RuntimeError(
            f"Move power mismatch for {move_name}: simulator={expected_power}, calc={normal_result['bp']}"
        )

    normal = normal_result["damage"]
    crit = crit_result["damage"]

    if not normal or not crit:
        raise RuntimeError("Calculator returned empty damage array")

    distribution = {}

    normal_weight = hit_prob * (1.0 - crit_prob)
    crit_weight = hit_prob * crit_prob

    for damage in normal:
        d = int(damage)
        distribution[d] = distribution.get(d, 0.0) + normal_weight / len(normal)

    for damage in crit:
        d = int(damage)
        distribution[d] = distribution.get(d, 0.0) + crit_weight / len(crit)

    miss_weight = 1.0 - hit_prob
    if miss_weight > 0.0:
        distribution[0] = distribution.get(0, 0.0) + miss_weight

    total = sum(distribution.values())
    if total <= 0.0:
        raise RuntimeError("Expected distribution sum is non-positive")

    for key in list(distribution.keys()):
        distribution[key] /= total

    return dict(sorted(distribution.items()))


def write_header(output_file, args):
    output_file.write("# damage_dataset_v1\n")
    output_file.write(f"# num_setups={args.num_setups}\n")
    output_file.write(f"# seed={args.seed}\n")
    output_file.write(
        "# format: attacker_id,move_id,defender_id|damage:probability,damage:probability,...\n"
    )


def generate_dataset(args):
    move_metadata = parse_move_metadata(ROOT / "data_sim" / "generated_movedex.h")
    speeds = parse_speed_table(ROOT / "data_sim" / "pokedex.h")
    damaging_moves = candidate_moves(move_metadata)

    if len(speeds) <= MAX_POKE_ID or len(POKEMON_NAMES) <= MAX_POKE_ID:
        raise RuntimeError("Species tables do not include the required Gen1 ID range")

    rng = random.Random(args.seed)
    max_attempts = max(args.num_setups * args.max_attempt_multiplier, args.num_setups)
    generated = 0
    skipped = 0

    args.output.parent.mkdir(parents=True, exist_ok=True)

    if args.calc_server_url:
        calc = CalcClient(base_url=args.calc_server_url)
    else:
        calc = CalcClient(
            server_script=args.calc_server_script, port=args.calc_server_port
        )
    calc.start()
    try:
        with args.output.open("w", encoding="utf-8") as out:
            write_header(out, args)

            attempts = 0
            while generated < args.num_setups and attempts < max_attempts:
                attempts += 1

                attacker_id = rng.randint(1, MAX_POKE_ID)
                defender_id = rng.randint(1, MAX_POKE_ID)
                move_id = rng.choice(damaging_moves)

                attacker_name = POKEMON_NAMES[attacker_id]
                defender_name = POKEMON_NAMES[defender_id]
                move_name = MOVE_LABELS[move_id]

                try:
                    hit_prob = calc_hit_prob(move_metadata[move_id].accuracy)
                    crit_prob = calc_crit_prob(speeds[attacker_id], move_id)

                    expected = build_expected_distribution(
                        calc,
                        attacker_name=attacker_name,
                        defender_name=defender_name,
                        move_name=move_name,
                        expected_power=move_metadata[move_id].power,
                        hit_prob=hit_prob,
                        crit_prob=crit_prob,
                    )
                except Exception:
                    skipped += 1
                    continue

                distribution = ",".join(
                    f"{damage}:{probability:.12f}"
                    for damage, probability in expected.items()
                )
                out.write(f"{attacker_id},{move_id},{defender_id}|{distribution}\n")
                generated += 1
    finally:
        calc.stop()

    if generated < args.num_setups:
        raise RuntimeError(
            f"Generated only {generated} setups out of requested {args.num_setups}. "
            f"Increase --max-attempt-multiplier or inspect naming compatibility."
        )

    print(f"Wrote {generated} setups to {args.output}")
    print(f"Skipped {skipped} failed candidate setups")


def main():
    args = parse_args()
    if args.num_setups <= 0:
        raise SystemExit("--num-setups must be > 0")
    if args.max_attempt_multiplier <= 0:
        raise SystemExit("--max-attempt-multiplier must be > 0")
    if args.calc_server_port <= 0:
        raise SystemExit("--calc-server-port must be > 0")
    generate_dataset(args)


if __name__ == "__main__":
    main()
