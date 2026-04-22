#!/bin/bash
set -e

IFACE="vcan0"

# CAN コアモジュールのロード（AF_CAN ソケットに必要）
for mod in can can_raw vcan; do
    if ! lsmod | grep -q "^${mod}"; then
        echo "[setup_vcan] Loading ${mod} module..."
        sudo modprobe "$mod" || { echo "[setup_vcan] ERROR: modprobe $mod failed"; exit 1; }
    fi
done

# vcan0 が既に存在する場合はスキップ
if ip link show "$IFACE" > /dev/null 2>&1; then
    echo "[setup_vcan] $IFACE already exists, skipping."
    exit 0
fi

# vcan0 の作成・起動
echo "[setup_vcan] Creating $IFACE..."
sudo ip link add "$IFACE" type vcan || { echo "[setup_vcan] ERROR: failed to add $IFACE"; exit 1; }

echo "[setup_vcan] Bringing up $IFACE..."
sudo ip link set "$IFACE" up || { echo "[setup_vcan] ERROR: failed to bring up $IFACE"; exit 1; }

echo "[setup_vcan] $IFACE is ready."
