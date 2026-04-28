#include "bms_ecu.h"
#include "can_protocol.h"

#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <chrono>
#include <thread>

static constexpr double WARN_THRESHOLD     = 20.0;
static constexpr double CRITICAL_THRESHOLD = 10.0;

BmsEcu::BmsEcu(int can_sock, double initial_pct, double drain_rate_per_sec)
    : sock_(can_sock), battery_pct_(initial_pct), drain_rate_(drain_rate_per_sec) {}

void BmsEcu::send_battery_status() {
    uint8_t flags = 0;
    if (battery_pct_ <= WARN_THRESHOLD)     flags |= BATTERY_FLAG_LOW;
    if (battery_pct_ <= CRITICAL_THRESHOLD) flags |= BATTERY_FLAG_CRITICAL;

    struct can_frame frame{};
    encode_battery_status(frame, battery_pct_, flags);

    if (write(sock_, &frame, sizeof(frame)) < 0) {
        perror("[BMS] write");
    }
}

void BmsEcu::handle_command(uint8_t cmd) {
    if (cmd == CMD_ACTIVATE) {
        draining_ = true;
        printf("[BMS] Draining started\n");
        fflush(stdout);
    } else if (cmd == CMD_DEACTIVATE || cmd == CMD_DRIVER_RETURNED) {
        draining_ = false;
        printf("[BMS] Draining stopped\n");
        fflush(stdout);
    }
}

void BmsEcu::run() {
    printf("[BMS] Started (battery=%.1f%%, drain=%.2f%%/s)\n", battery_pct_, drain_rate_);
    fflush(stdout);

    struct can_filter filter{};
    filter.can_id   = CAN_ID_DOG_MODE_CMD;
    filter.can_mask = CAN_SFF_MASK;
    setsockopt(sock_, SOL_CAN_RAW, CAN_RAW_FILTER, &filter, sizeof(filter));

    struct timeval tv{0, 100000}; // 100ms timeout
    setsockopt(sock_, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    auto last_cycle = std::chrono::steady_clock::now();
    auto last_send  = last_cycle;

    while (running_) {
        struct can_frame frame{};
        ssize_t nbytes = read(sock_, &frame, sizeof(frame));
        if (nbytes > 0 && (frame.can_id & CAN_EFF_MASK) == CAN_ID_DOG_MODE_CMD) {
            handle_command(decode_cmd(frame));
        }

        auto now = std::chrono::steady_clock::now();
        double dt = std::chrono::duration<double>(now - last_cycle).count();
        last_cycle = now;

        if (draining_ && battery_pct_ > 0) {
            battery_pct_ -= drain_rate_ * dt;
            if (battery_pct_ < 0) battery_pct_ = 0;
        }

        if (!warned_low_ && battery_pct_ <= WARN_THRESHOLD) {
            warned_low_ = true;
            printf("[BMS] Battery LOW (%.1f%%)\n", battery_pct_);
            fflush(stdout);
        }
        if (!warned_critical_ && battery_pct_ <= CRITICAL_THRESHOLD) {
            warned_critical_ = true;
            printf("[BMS] Battery CRITICAL (%.1f%%)\n", battery_pct_);
            fflush(stdout);
        }

        double since_send = std::chrono::duration<double>(now - last_send).count();
        if (since_send >= 1.0) {
            send_battery_status();
            last_send = now;
        }
    }

    printf("[BMS] Stopped\n");
    fflush(stdout);
}
