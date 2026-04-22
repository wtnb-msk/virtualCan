#!/usr/bin/env python3
import signal
import sys
import can

IFACE = "vcan0"
TARGET_ID = 0x123


def judge(msg: can.Message) -> str:
    if len(msg.data) == 0:
        return "NORMAL"
    if all(b == 0xFF for b in msg.data):
        return "ABNORMAL"
    return "NORMAL"


def main():
    try:
        bus = can.Bus(channel=IFACE, interface="socketcan")
    except OSError as e:
        print(f"[ECU] ERROR: Cannot open {IFACE}: {e}", file=sys.stderr)
        sys.exit(1)

    def shutdown(signum, frame):
        print("[ECU] Shutting down...")
        bus.shutdown()
        sys.exit(0)

    signal.signal(signal.SIGTERM, shutdown)
    signal.signal(signal.SIGINT, shutdown)

    print(f"[ECU] Listening on {IFACE} ...")

    for msg in bus:
        if msg.arbitration_id != TARGET_ID:
            continue
        label = judge(msg)
        print(f"[{label}] ID=0x{msg.arbitration_id:03X} Data={msg.data.hex()}")
        sys.stdout.flush()


if __name__ == "__main__":
    main()
