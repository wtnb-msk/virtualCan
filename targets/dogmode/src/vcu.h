#ifndef VCU_H
#define VCU_H

#include "state_machine.h"
#include <atomic>
#include <cstdint>

class Vcu {
public:
    Vcu(int can_sock);

    void run();
    void stop() { running_ = false; }
    bool is_terminated() const { return sm_.is_terminated(); }

private:
    int sock_;
    StateMachine sm_;
    std::atomic<bool> running_{true};
    uint8_t target_temp_{22};
    double last_battery_pct_{0};

    void handle_dog_mode_cmd(uint8_t cmd, uint8_t target_temp);
    void handle_battery_status(double pct, uint8_t flags);
    void handle_temp_status(int16_t temp_c100);

    void do_transition(TransitionTrigger trigger, uint8_t reason);
    void broadcast_state(uint8_t prev_state, uint8_t reason);
    void initiate_safe_stop(uint8_t reason);
    void complete_safe_stop();
};

#endif
