#include <chrono>
#include <memory>
#include <thread>
#include <iostream>

#include "lib/logging.hpp"
#include "dmx/ftdi_universe.hpp"

int main(int argc, char* argv[]) {
    spdlog::set_level(spdlog::level::debug);
    try {
        dmxfish::dmx::ftdi_universe u(-1, 0x0403, 0x6001, "", "");
	std::cout << "Device connected. Lamp test." << std::endl;
	u[0] = 255;
	u[1] = 255;
	u[2] = 255;
	u[3] = 255;
	u.send_data();
	std::this_thread::sleep_for(std::chrono::milliseconds(1500));
	std::cout << "Phase 1." << std::endl;
        for (uint16_t i = 0; i < 16384; i++) {
            u[1] = (i >> 8) & 0xff;
            u[2] = i & 0xff;
            u.send_data();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
	std::cout << "Phase 2." << std::endl;
        for (uint16_t i = 16384; i > 0; i++) {
            u[1] = (i >> 8) & 0xff;
            u[2] = i & 0xff;
            u.send_data();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        u[1] = 0;
        u[2] = 0;
	std::cout << "Phase 3." << std::endl;
        for (uint16_t i = 0; i < 16384; i++) {
            u[3] = (i >> 8) & 0xff;
            u[4] = i & 0xff;
            u.send_data();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
	std::cout << "Phase 4." << std::endl;
        for (uint16_t i = 16384; i > 0; i++) {
            u[3] = (i >> 8) & 0xff;
            u[4] = i & 0xff;
            u.send_data();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        u[3] = 0;
        u[4] = 0;
	std::cout << "Done." << std::endl;
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        return 1;
    }
    return 0;
}
