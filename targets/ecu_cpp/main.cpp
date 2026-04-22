#include <cstdio>
#include <cstring>
#include <csignal>
#include <unistd.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/can.h>
#include <linux/can/raw.h>

static volatile bool running = true;

static void handle_signal(int) {
    running = false;
}

static const canid_t TARGET_ID = 0x123;

static bool is_abnormal(const struct can_frame &frame) {
    if (frame.can_dlc == 0) return false;
    for (int i = 0; i < frame.can_dlc; ++i) {
        if (frame.data[i] != 0xFF) return false;
    }
    return true;
}

int main() {
    signal(SIGTERM, handle_signal);
    signal(SIGINT,  handle_signal);

    int sock = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (sock < 0) {
        perror("[ecu_cpp] socket");
        return 1;
    }

    struct ifreq ifr;
    strncpy(ifr.ifr_name, "vcan0", IFNAMSIZ - 1);
    if (ioctl(sock, SIOCGIFINDEX, &ifr) < 0) {
        perror("[ecu_cpp] ioctl SIOCGIFINDEX");
        return 1;
    }

    struct sockaddr_can addr{};
    addr.can_family  = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("[ecu_cpp] bind");
        return 1;
    }

    struct timeval tv{1, 0};
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    while (running) {
        struct can_frame frame{};
        ssize_t nbytes = read(sock, &frame, sizeof(frame));
        if (nbytes <= 0) continue;

        if ((frame.can_id & CAN_EFF_MASK) != TARGET_ID) continue;

        char hex[17] = {};
        for (int i = 0; i < frame.can_dlc; ++i)
            snprintf(hex + i * 2, 3, "%02x", frame.data[i]);

        const char *label = is_abnormal(frame) ? "ABNORMAL" : "NORMAL";
        printf("[%s] ID=0x%03X Data=%s\n", label, frame.can_id & CAN_EFF_MASK, hex);
        fflush(stdout);
    }

    close(sock);
    return 0;
}
