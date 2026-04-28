#include "vcu.h"
#include "can_protocol.h"

#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <thread>

static void can_send(int sock, struct can_frame &frame) {
    if (write(sock, &frame, sizeof(frame)) < 0) {
        perror("[VCU] write");
    }
}

Vcu::Vcu(int can_sock) : sock_(can_sock) {}

void Vcu::broadcast_state(uint8_t prev_state, uint8_t reason) {
    struct can_frame frame{};
    encode_state_broadcast(frame, static_cast<uint8_t>(sm_.current_state()), prev_state, reason);
    can_send(sock_, frame);
}

void Vcu::initiate_safe_stop(uint8_t reason) {
    struct can_frame frame{};
    encode_safe_stop_cmd(frame, SAFE_STOP_INITIATED, reason);
    can_send(sock_, frame);
}

void Vcu::complete_safe_stop() {
    struct can_frame frame{};
    encode_safe_stop_cmd(frame, SAFE_STOP_COMPLETED, REASON_NONE);
    can_send(sock_, frame);
}

void Vcu::do_transition(TransitionTrigger trigger, uint8_t reason) {
    VehicleState prev = sm_.current_state();
    if (sm_.process_trigger(trigger)) {
        printf("[VCU] State: %s -> %s (trigger=%s)\n",
               state_name(prev), state_name(sm_.current_state()), trigger_name(trigger));
        fflush(stdout);
        broadcast_state(static_cast<uint8_t>(prev), reason);
    }
}

void Vcu::handle_dog_mode_cmd(uint8_t cmd, uint8_t target_temp) {
    if (cmd == CMD_ACTIVATE) {
        target_temp_ = target_temp;
        printf("[VCU] Activate Dog Mode (target=%d C)\n", target_temp_);
        fflush(stdout);
        do_transition(TransitionTrigger::ACTIVATE, REASON_NONE);
    } else if (cmd == CMD_DEACTIVATE) {
        printf("[VCU] Deactivate Dog Mode\n");
        fflush(stdout);
        do_transition(TransitionTrigger::DEACTIVATE, REASON_APP_STOP);
    } else if (cmd == CMD_DRIVER_RETURNED) {
        printf("[VCU] Driver returned\n");
        fflush(stdout);
        do_transition(TransitionTrigger::DRIVER_RETURNED, REASON_DRIVER_RETURN);
    }
}

void Vcu::handle_battery_status(double pct, uint8_t flags) {
    last_battery_pct_ = pct;

    VehicleState st = sm_.current_state();
    if (st != VehicleState::ACTIVE && st != VehicleState::LOW_BATTERY)
        return;

    if (flags & BATTERY_FLAG_CRITICAL) {
        printf("[VCU] Battery critical (%.1f%%), initiating safe stop\n", pct);
        fflush(stdout);
        initiate_safe_stop(REASON_BATTERY_CRITICAL);
        do_transition(TransitionTrigger::BATTERY_CRITICAL, REASON_BATTERY_CRITICAL);

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        complete_safe_stop();
        do_transition(TransitionTrigger::SAFE_STOP_COMPLETE, REASON_BATTERY_CRITICAL);
    } else if (flags & BATTERY_FLAG_LOW) {
        do_transition(TransitionTrigger::BATTERY_LOW, REASON_NONE);
    }
}

void Vcu::handle_temp_status(int16_t temp_c100) {
    double temp_c = temp_c100 / 100.0;
    printf("[VCU] Temp: %.1f C (target=%d C)\n", temp_c, target_temp_);
    fflush(stdout);
}

void Vcu::run() {
    printf("[VCU] Started (state=%s)\n", state_name(sm_.current_state()));
    fflush(stdout);

    struct can_filter filters[3];
    filters[0].can_id = CAN_ID_DOG_MODE_CMD;   filters[0].can_mask = CAN_SFF_MASK;
    filters[1].can_id = CAN_ID_BATTERY_STATUS;  filters[1].can_mask = CAN_SFF_MASK;
    filters[2].can_id = CAN_ID_TEMP_STATUS;     filters[2].can_mask = CAN_SFF_MASK;
    setsockopt(sock_, SOL_CAN_RAW, CAN_RAW_FILTER, filters, sizeof(filters));

    struct timeval tv{0, 100000}; // 100ms timeout
    setsockopt(sock_, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    while (running_ && !sm_.is_terminated()) {
        struct can_frame frame{};
        ssize_t nbytes = read(sock_, &frame, sizeof(frame));
        if (nbytes <= 0) continue;

        canid_t id = frame.can_id & CAN_EFF_MASK;

        if (id == CAN_ID_DOG_MODE_CMD) {
            handle_dog_mode_cmd(decode_cmd(frame), decode_target_temp(frame));
        } else if (id == CAN_ID_BATTERY_STATUS) {
            handle_battery_status(decode_battery_pct(frame), decode_battery_flags(frame));
        } else if (id == CAN_ID_TEMP_STATUS) {
            handle_temp_status(decode_temp_c100(frame));
        }
    }

    printf("[VCU] State: %s\n", state_name(sm_.current_state()));
    printf("[VCU] Stopped\n");
    fflush(stdout);
}
