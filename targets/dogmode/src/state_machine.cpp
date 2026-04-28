#include "state_machine.h"

const char* state_name(VehicleState s) {
    switch (s) {
        case VehicleState::STANDBY:       return "STANDBY";
        case VehicleState::ACTIVE:        return "ACTIVE";
        case VehicleState::LOW_BATTERY:   return "LOW_BATTERY";
        case VehicleState::SAFE_STOP_SEQ: return "SAFE_STOP";
        case VehicleState::TERMINATED:    return "TERMINATED";
    }
    return "UNKNOWN";
}

const char* trigger_name(TransitionTrigger t) {
    switch (t) {
        case TransitionTrigger::ACTIVATE:           return "ACTIVATE";
        case TransitionTrigger::DEACTIVATE:         return "DEACTIVATE";
        case TransitionTrigger::DRIVER_RETURNED:    return "DRIVER_RETURNED";
        case TransitionTrigger::BATTERY_LOW:        return "BATTERY_LOW";
        case TransitionTrigger::BATTERY_CRITICAL:   return "BATTERY_CRITICAL";
        case TransitionTrigger::SAFE_STOP_COMPLETE: return "SAFE_STOP_COMPLETE";
    }
    return "UNKNOWN";
}

StateMachine::StateMachine() : state_(VehicleState::STANDBY) {}

bool StateMachine::process_trigger(TransitionTrigger trigger) {
    VehicleState next = state_;

    switch (state_) {
        case VehicleState::STANDBY:
            if (trigger == TransitionTrigger::ACTIVATE)
                next = VehicleState::ACTIVE;                    // TR-001
            break;

        case VehicleState::ACTIVE:
            if (trigger == TransitionTrigger::BATTERY_LOW)
                next = VehicleState::LOW_BATTERY;               // TR-002
            else if (trigger == TransitionTrigger::DRIVER_RETURNED)
                next = VehicleState::TERMINATED;                // TR-003
            else if (trigger == TransitionTrigger::DEACTIVATE)
                next = VehicleState::TERMINATED;                // TR-003b
            else if (trigger == TransitionTrigger::BATTERY_CRITICAL)
                next = VehicleState::SAFE_STOP_SEQ;             // TR-005b
            break;

        case VehicleState::LOW_BATTERY:
            if (trigger == TransitionTrigger::DRIVER_RETURNED)
                next = VehicleState::TERMINATED;                // TR-004
            else if (trigger == TransitionTrigger::DEACTIVATE)
                next = VehicleState::TERMINATED;                // TR-004b
            else if (trigger == TransitionTrigger::BATTERY_CRITICAL)
                next = VehicleState::SAFE_STOP_SEQ;             // TR-005
            break;

        case VehicleState::SAFE_STOP_SEQ:
            if (trigger == TransitionTrigger::SAFE_STOP_COMPLETE)
                next = VehicleState::TERMINATED;                // TR-006
            break;

        case VehicleState::TERMINATED:
            break;
    }

    if (next != state_) {
        state_ = next;
        return true;
    }
    return false;
}
