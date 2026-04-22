#!/bin/bash
set -e

LOG_FILE="${1:-logs/ecu.log}"

if [ ! -f "$LOG_FILE" ]; then
    echo "FAIL: log file not found: $LOG_FILE"
    exit 1
fi

grep -q '\[NORMAL\]' "$LOG_FILE"   || { echo "FAIL: [NORMAL] not found in $LOG_FILE";   exit 1; }
grep -q '\[ABNORMAL\]' "$LOG_FILE" || { echo "FAIL: [ABNORMAL] not found in $LOG_FILE"; exit 1; }

echo "PASS: [NORMAL] and [ABNORMAL] both detected."
