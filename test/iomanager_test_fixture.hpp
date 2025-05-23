//
// Created by leondietrich on 2/15/25.
// SPDX-License-Identifier: GPL-3.0-or-later
//
#pragma once

#include <boost/test/included/unit_test.hpp>
#include <filesystem>

#include "main.hpp"
#include "lib/logging.hpp"

struct gf {
    bool constructed = true;
    gf() {
        if (std::filesystem::exists("/tmp/fish.sock")) {
            ::spdlog::warn("Found existing socket. Maybe from a rouge test? Attempting to remove it.");
            std::filesystem::remove("/tmp/fish.sock");
        }
        if(std::filesystem::exists("/tmp/fish.sock")) {
            constructed = false;
            ::spdlog::warn("Skipped manager init");
        } else {
            construct_managers();
            ::spdlog::info("Created managers.");
        }
    }
    ~gf() {
        if(constructed) {
            destruct_managers();
            ::spdlog::info("Stopped managers.");
        }
    }
};

BOOST_TEST_GLOBAL_FIXTURE(gf);
