#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
TARGET="${1:-$SCRIPT_DIR/targets/ecu_basic}"
TARGET="$(cd "$TARGET" && pwd)"

SCENARIO="$TARGET/scenario.yml"
LOG_DIR="$SCRIPT_DIR/logs"
ECU_LOG="$LOG_DIR/ecu.log"
CANDUMP_LOG="$LOG_DIR/candump.log"
VENV="$SCRIPT_DIR/.venv/bin/python3"

# Python アプリ (app.py) またはコンパイル済みバイナリ (app) を自動判別
if [ -f "$TARGET/app.py" ]; then
    APP_TYPE="python"
    APP="$TARGET/app.py"
elif [ -f "$TARGET/app" ]; then
    APP_TYPE="binary"
    APP="$TARGET/app"
else
    APP_TYPE=""
    APP=""
fi

ECU_PID=""
CANDUMP_PID=""

cleanup() {
    echo ""
    echo "[run_poc] Stopping processes..."
    if [ -n "$ECU_PID" ]; then kill "$ECU_PID" 2>/dev/null; wait "$ECU_PID" 2>/dev/null; fi
    if [ -n "$CANDUMP_PID" ]; then kill "$CANDUMP_PID" 2>/dev/null; wait "$CANDUMP_PID" 2>/dev/null; fi
    echo ""
    echo "=== candump log ==="
    cat "$CANDUMP_LOG" 2>/dev/null || echo "(empty)"
    echo ""
    echo "=== ECU log ==="
    cat "$ECU_LOG" 2>/dev/null || echo "(empty)"
}
trap cleanup EXIT

mkdir -p "$LOG_DIR"

# 引数バリデーション
if [ -z "$APP" ]; then
    echo "[run_poc] ERROR: app.py or app binary not found in target: $TARGET"
    exit 1
fi
if [ ! -f "$SCENARIO" ]; then
    echo "[run_poc] ERROR: scenario.yml not found in target: $TARGET"
    exit 1
fi

echo "[run_poc] Target: $TARGET"

# --- 1. vcan0 セットアップ ---
echo "[run_poc] Setting up vcan0..."
bash "$SCRIPT_DIR/scripts/setup_vcan.sh" || { echo "[run_poc] ERROR: vcan setup failed"; exit 1; }

# --- 2. ECU アプリ起動 ---
echo "[run_poc] Starting ECU app ($APP_TYPE)..."
if [ "$APP_TYPE" = "python" ]; then
    if [ ! -f "$VENV" ]; then
        echo "[run_poc] ERROR: .venv not found. Run: python3 -m venv .venv && pip install -r requirements.txt"
        exit 2
    fi
    "$VENV" "$APP" > "$ECU_LOG" 2>&1 &
else
    "$APP" > "$ECU_LOG" 2>&1 &
fi
ECU_PID=$!
sleep 0.5
if ! kill -0 "$ECU_PID" 2>/dev/null; then
    echo "[run_poc] ERROR: ECU app failed to start. See $ECU_LOG"
    exit 2
fi
echo "[run_poc] ECU app started (PID=$ECU_PID)"

# --- 3. candump 起動 ---
echo "[run_poc] Starting candump..."
candump vcan0 > "$CANDUMP_LOG" 2>&1 &
CANDUMP_PID=$!
sleep 0.3
echo "[run_poc] candump started (PID=$CANDUMP_PID)"

# --- 4. シナリオ実行 ---
echo "[run_poc] Running scenario..."
bash "$SCRIPT_DIR/scripts/run_scenario.sh" "$SCENARIO"

echo "[run_poc] All steps done."
