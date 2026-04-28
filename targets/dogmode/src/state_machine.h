#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <cstdint>

enum class VehicleState : uint8_t {
    STANDBY       = 1,
    ACTIVE        = 2,
    LOW_BATTERY   = 3,
    SAFE_STOP_SEQ = 4,
    TERMINATED    = 5
};

enum class TransitionTrigger {
    ACTIVATE,
    DEACTIVATE,
    DRIVER_RETURNED,
    BATTERY_LOW,
    BATTERY_CRITICAL,
    SAFE_STOP_COMPLETE
};

const char* state_name(VehicleState s);
const char* trigger_name(TransitionTrigger t);

class StateMachine {
public:
    StateMachine();

    VehicleState current_state() const { return state_; }
    bool is_terminated() const { return state_ == VehicleState::TERMINATED; }

    // Returns true if a transition occurred
    bool process_trigger(TransitionTrigger trigger);

private:
    VehicleState state_;
};

#endif
