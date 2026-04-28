#ifndef CAN_PROTOCOL_H
#define CAN_PROTOCOL_H

#include <cstdint>
#include <linux/can.h>

// --- CAN IDs ---
static constexpr canid_t CAN_ID_DOG_MODE_CMD   = 0x100;
static constexpr canid_t CAN_ID_STATE_BROADCAST = 0x101;
static constexpr canid_t CAN_ID_SAFE_STOP_CMD   = 0x110;
static constexpr canid_t CAN_ID_TEMP_STATUS     = 0x200;
static constexpr canid_t CAN_ID_BATTERY_STATUS  = 0x300;

// --- DOG_MODE_CMD (0x100) commands ---
static constexpr uint8_t CMD_ACTIVATE        = 0x01;
static constexpr uint8_t CMD_DEACTIVATE      = 0x02;
static constexpr uint8_t CMD_DRIVER_RETURNED = 0x03;

// --- STATE_BROADCAST (0x101) state IDs ---
static constexpr uint8_t STATE_STANDBY       = 1;
static constexpr uint8_t STATE_ACTIVE        = 2;
static constexpr uint8_t STATE_LOW_BATTERY   = 3;
static constexpr uint8_t STATE_SAFE_STOP_SEQ = 4;
static constexpr uint8_t STATE_TERMINATED    = 5;

// --- STATE_BROADCAST (0x101) reason codes ---
static constexpr uint8_t REASON_NONE             = 0x00;
static constexpr uint8_t REASON_DRIVER_RETURN    = 0x01;
static constexpr uint8_t REASON_APP_STOP         = 0x02;
static constexpr uint8_t REASON_BATTERY_CRITICAL = 0x03;
static constexpr uint8_t REASON_TIMEOUT          = 0x04;

// --- SAFE_STOP_CMD (0x110) phases ---
static constexpr uint8_t SAFE_STOP_INITIATED = 0x01;
static constexpr uint8_t SAFE_STOP_COMPLETED = 0x02;

// --- BATTERY_STATUS (0x300) flags ---
static constexpr uint8_t BATTERY_FLAG_LOW      = 0x01;
static constexpr uint8_t BATTERY_FLAG_CRITICAL  = 0x02;

// --- Encode helpers ---

inline void encode_battery_status(struct can_frame &frame, double pct, uint8_t flags) {
    frame.can_id = CAN_ID_BATTERY_STATUS;
    frame.can_dlc = 8;
    uint16_t pct_x10 = static_cast<uint16_t>(pct * 10);
    frame.data[0] = (pct_x10 >> 8) & 0xFF;
    frame.data[1] = pct_x10 & 0xFF;
    frame.data[2] = flags;
    for (int i = 3; i < 8; ++i) frame.data[i] = 0;
}

inline void encode_state_broadcast(struct can_frame &frame, uint8_t state_id, uint8_t prev_state, uint8_t reason) {
    frame.can_id = CAN_ID_STATE_BROADCAST;
    frame.can_dlc = 8;
    frame.data[0] = state_id;
    frame.data[1] = prev_state;
    frame.data[2] = reason;
    for (int i = 3; i < 8; ++i) frame.data[i] = 0;
}

inline void encode_safe_stop_cmd(struct can_frame &frame, uint8_t phase, uint8_t reason) {
    frame.can_id = CAN_ID_SAFE_STOP_CMD;
    frame.can_dlc = 8;
    frame.data[0] = phase;
    frame.data[1] = reason;
    for (int i = 2; i < 8; ++i) frame.data[i] = 0;
}

// --- Decode helpers ---

inline uint8_t decode_cmd(const struct can_frame &frame) {
    return frame.data[0];
}

inline uint8_t decode_target_temp(const struct can_frame &frame) {
    return frame.data[1];
}

inline double decode_battery_pct(const struct can_frame &frame) {
    uint16_t pct_x10 = (static_cast<uint16_t>(frame.data[0]) << 8) | frame.data[1];
    return pct_x10 / 10.0;
}

inline uint8_t decode_battery_flags(const struct can_frame &frame) {
    return frame.data[2];
}

inline int16_t decode_temp_c100(const struct can_frame &frame) {
    return static_cast<int16_t>((static_cast<uint16_t>(frame.data[0]) << 8) | frame.data[1]);
}

#endif
