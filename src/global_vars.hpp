#pragma once

#include <chrono>

std::chrono::time_point<std::chrono::system_clock> get_start_time();

void reset_start_time();