#!/bin/bash
set -e

LOG_FILE="${1:-logs/ecu.log}"
SCENARIO_FILE="${2:-}"
VENV_PYTHON="$(dirname "$0")/../.venv/bin/python3"

if [ ! -f "$LOG_FILE" ]; then
    echo "FAIL: log file not found: $LOG_FILE"
    exit 1
fi

# scenario.yml が渡されなければ従来の固定アサート
if [ -z "$SCENARIO_FILE" ]; then
    grep -q '\[NORMAL\]'   "$LOG_FILE" || { echo "FAIL: [NORMAL] not found in $LOG_FILE";   exit 1; }
    grep -q '\[ABNORMAL\]' "$LOG_FILE" || { echo "FAIL: [ABNORMAL] not found in $LOG_FILE"; exit 1; }
    echo "PASS: [NORMAL] and [ABNORMAL] both detected."
    exit 0
fi

if [ ! -f "$SCENARIO_FILE" ]; then
    echo "FAIL: scenario file not found: $SCENARIO_FILE"
    exit 1
fi

"$VENV_PYTHON" - "$LOG_FILE" "$SCENARIO_FILE" << 'EOF'
import sys
import yaml

log_file     = sys.argv[1]
scenario_file = sys.argv[2]

with open(log_file) as f:
    log = f.read()

with open(scenario_file) as f:
    scenario = yaml.safe_load(f)

assertions = scenario.get("assertions", [])
if not assertions:
    print("SKIP: no assertions defined in scenario.")
    sys.exit(0)

failed = False
for assertion in assertions:
    pattern     = assertion["pattern"]
    description = assertion.get("description", pattern)
    if pattern in log:
        print(f"PASS: {description}")
    else:
        print(f"FAIL: {description}  (pattern not found: {pattern!r})")
        failed = True

sys.exit(1 if failed else 0)
EOF
