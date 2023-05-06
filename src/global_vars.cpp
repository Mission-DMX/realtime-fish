#include "global_vars.hpp"

static std::chrono::time_point<std::chrono::system_clock> start_time = std::chrono::system_clock::now();

std::chrono::time_point<std::chrono::system_clock> get_start_time(){
    return start_time;
}

void reset_start_time(){
    start_time = std::chrono::system_clock::now();
}