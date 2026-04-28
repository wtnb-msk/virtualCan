#include "vcu.h"
#include "bms_ecu.h"

#include <cstdio>
#include <cstring>
#include <csignal>
#include <unistd.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <thread>

static constexpr double INITIAL_BATTERY_PCT    = 80.0;
static constexpr double DRAIN_RATE_PER_SEC     = 5.0; // CI用高速ドレイン: 約14秒で80%→0%

static Vcu* g_vcu = nullptr;
static BmsEcu* g_bms = nullptr;

static void handle_signal(int) {
    if (g_vcu) g_vcu->stop();
    if (g_bms) g_bms->stop();
}

static int create_can_socket(const char* iface) {
    int sock = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (sock < 0) {
        perror("[main] socket");
        return -1;
    }

    struct ifreq ifr{};
    strncpy(ifr.ifr_name, iface, IFNAMSIZ - 1);
    if (ioctl(sock, SIOCGIFINDEX, &ifr) < 0) {
        perror("[main] ioctl");
        close(sock);
        return -1;
    }

    struct sockaddr_can addr{};
    addr.can_family  = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("[main] bind");
        close(sock);
        return -1;
    }

    return sock;
}

int main() {
    signal(SIGTERM, handle_signal);
    signal(SIGINT,  handle_signal);

    printf("[main] Dog Mode ECU starting...\n");
    fflush(stdout);

    int vcu_sock = create_can_socket("vcan0");
    int bms_sock = create_can_socket("vcan0");
    if (vcu_sock < 0 || bms_sock < 0) return 1;

    Vcu vcu(vcu_sock);
    BmsEcu bms(bms_sock, INITIAL_BATTERY_PCT, DRAIN_RATE_PER_SEC);
    g_vcu = &vcu;
    g_bms = &bms;

    std::thread bms_thread([&bms]() { bms.run(); });
    std::thread vcu_thread([&vcu]() { vcu.run(); });

    vcu_thread.join();

    bms.stop();
    bms_thread.join();

    close(vcu_sock);
    close(bms_sock);

    printf("[main] Dog Mode ECU terminated.\n");
    fflush(stdout);
    return 0;
}
