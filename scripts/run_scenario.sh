#!/bin/bash
set -e

IFACE="vcan0"
SCENARIO_FILE="$1"
VENV_PYTHON="$(dirname "$0")/../.venv/bin/python3"

if [ -z "$SCENARIO_FILE" ]; then
    echo "[run_scenario] ERROR: scenario file not specified."
    exit 1
fi

if [ ! -f "$SCENARIO_FILE" ]; then
    echo "[run_scenario] ERROR: scenario file not found: $SCENARIO_FILE"
    exit 1
fi

if ! ip link show "$IFACE" > /dev/null 2>&1; then
    echo "[run_scenario] ERROR: $IFACE not found. Run setup_vcan.sh first."
    exit 1
fi

echo "[run_scenario] Running scenario: $SCENARIO_FILE"

"$VENV_PYTHON" - "$IFACE" "$SCENARIO_FILE" << 'EOF'
import sys
import time
import subprocess
import yaml

iface = sys.argv[1]
scenario_file = sys.argv[2]

with open(scenario_file) as f:
    scenario = yaml.safe_load(f)

print(f"[run_scenario] Scenario: {scenario['name']}")

for step in scenario['steps']:
    label    = step['label']
    template = step['template']
    count    = int(step['count'])
    interval = float(step['interval'])

    print(f"[run_scenario] Step: {label} ({count} times, {interval}s interval)")
    for i in range(1, count + 1):
        subprocess.run(["cansend", iface, template], check=True)
        print(f"[run_scenario]   [{i}/{count}] {template}")
        if i < count:
            time.sleep(interval)

print("[run_scenario] Done.")
EOF
