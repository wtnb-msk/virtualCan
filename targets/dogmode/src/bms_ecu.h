#ifndef BMS_ECU_H
#define BMS_ECU_H

#include <atomic>

class BmsEcu {
public:
    BmsEcu(int can_sock, double initial_pct, double drain_rate_per_sec);

    void run();
    void stop() { running_ = false; }

private:
    int sock_;
    double battery_pct_;
    double drain_rate_;
    std::atomic<bool> running_{true};
    std::atomic<bool> draining_{false};
    bool warned_low_{false};
    bool warned_critical_{false};

    void send_battery_status();
    void handle_command(uint8_t cmd);
};

#endif
