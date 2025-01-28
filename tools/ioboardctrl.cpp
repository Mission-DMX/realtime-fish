//
// Created by Leon Dietrich on 23.01.25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#include <cstdint>
#include <iostream>
#include <memory>
#include <optional>
#include <string>

#include "dmx/ioboard_universe.hpp"
#include "io/ioboard/ioboard.hpp"

enum class action : uint8_t {
    HELP,
    WRITE_UNIVERSE,
    PRINT_CHIP_INFO
};

action selected_action = action::HELP;
dmxfish::io::ioboard_port_id_t port = 0;
std::string driver_path = "/dev/ft60x0";
std::shared_ptr<dmxfish::dmx::ioboard_universe> data;
std::shared_ptr<dmxfish::io::ioboard> board;

void ensure_board() {
    if(board == nullptr) {
        try {
            board = std::make_shared<dmxfish::io::ioboard>(driver_path);
        } catch (const rmrf::net::netio_exception& e) {
            std::cerr << "Failed to connect to io board: " << e.what() << std::endl;
            std::cerr << "Is the kernel driver properly loaded?" << std::endl;
            exit(1);
        }
    }
}

bool parse_cmd_args(int argc, char* argv[]) {
    for (int i = 0; i < argc; i++) {
        if (std::string s(argv[i]); s == "--help") {
            selected_action = action::HELP;
            break;
        } else if (s == "--interface") {
            if (i == argc) {
                std::cerr << "Expected driver path." << std::endl;
                return false;
            }
            driver_path = std::string(argv[++i]);
        } else if (s == "--info") {
            selected_action = action::PRINT_CHIP_INFO;
        } else if(s == "--write-universe") {
            if (++i == argc) {
                std::cerr << "Expected dmx port id." << std::endl;
                return false;
            }
            port = std::stoi(std::string(argv[i]));
            // TODO migrate universe data if port changed
        } else if (s.starts_with("--data=")) {
            auto channel = 0;
            auto cdata = 0;
            bool parsing_data = false;

            if(data == nullptr) {
                ensure_board();
                data = board->get_or_create_universe(port, (int) port).lock();
            }

            for (size_t spos = 6; spos < s.length(); spos++) {
                if (const auto& c = s[spos]; c == ',') {
                    parsing_data = false;
                    data->operator[](channel) = cdata;
                    channel = 0;
                    cdata = 0;
                } else if (c == ':') {
                    parsing_data = true;
                } else {
                    if (c > 57 || c < 48) {
                        std::cerr << "Expected number literal in data statement." << std::endl;
                        return false;
                    }
                    auto& selected_number = parsing_data ? cdata : channel;
                    selected_number *= 10;
                    selected_number += c - 48;
                }
            }
        }
    }
    return true;
}

void print_help() {
    std::cout << "--help\tprint this help" << std::endl;
    std::cout << "--info\tprint chip info and exit" << std::endl;
    std::cout << "--write-universe <port on device>\tselect the dmx port" << std::endl;
    std::cout << "--interface <path to interface>\tSet the driver interface to use. Defaults to " << driver_path <<
        "This needs to be set as the first argument." << std::endl;
    std::cout << "--data=<channel1>:<data1>,<channel2>:<data2>,...\tWrite data to selected port" << std::endl;
}

int main(int argc, char* argv[]) {
    if(!parse_cmd_args(argc, argv)) {
        print_help();
        return 1;
    }
    if (selected_action == action::HELP) {
        print_help();
        return 0;
    } else if (selected_action == action::WRITE_UNIVERSE) {
        ensure_board();
        board->transmit_universe(port);
    } else if (selected_action == action::PRINT_CHIP_INFO) {
        // TODO
    }
}
