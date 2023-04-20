#include <chrono>
#include <memory>
#include <thread>
#include <iostream>

#include "dmx/ftdi_universe.hpp"

int main(int argc, char* argv[]) {
    try {
        dmxfish::dmx::ftdi_universe u(-1, 0x0403, 0x6001, "", "");
        for (uint16_t i = 0; i < 16384; i++) {
            u[0] = (i >> 8) & 0xff;
            u[1] = i & 0xff;
            u.send_data();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        for (uint16_t i = 16384; i > 0; i++) {
            u[0] = (i >> 8) & 0xff;
            u[1] = i & 0xff;
            u.send_data();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        u[0] = 0;
        u[1] = 0;
        for (uint16_t i = 0; i < 16384; i++) {
            u[2] = (i >> 8) & 0xff;
            u[3] = i & 0xff;
            u.send_data();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        for (uint16_t i = 16384; i > 0; i++) {
            u[2] = (i >> 8) & 0xff;
            u[3] = i & 0xff;
            u.send_data();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        u[2] = 0;
        u[3] = 0;
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        return 1;
    }
    return 0;
}
