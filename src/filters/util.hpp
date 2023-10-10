#pragma once

/*
 * A class for some generic calculation
 */

#include <functional>
#include "filters/filter.hpp"
#include <string>
#include "dmx/pixel.hpp"
#include <vector>

namespace dmxfish::filters {
//    COMPILER_SUPRESS("-Weffc++")


    class util{
    public:
        static double float_zero;
        static double float_one;
        static double float_180;
        static double float_1000;
        static uint8_t low_8bit;
        static uint8_t high_8bit;
        static uint16_t low_16bit;
        static uint16_t high_16bit;
        static dmxfish::dmx::pixel color_white;
        static int count_occurence_of(const std::string &base_string, std::string pattern, size_t start, size_t end);
        static void init_mapping(
                const std::string &mappingstr,
                std::function<void(int)> reserve_space_8bit,
                std::function<void(int)> reserve_space_16bit,
                std::function<void(int)> reserve_space_float,
                std::function<void(int)> reserve_space_color,
                std::function<void(std::string&)> init_values_8bit,
                std::function<void(std::string&)> init_values_16bit,
                std::function<void(std::string&)> init_values_float,
                std::function<void(std::string&)> init_values_color
                );
    };

//    COMPILER_RESTORE("-Weffc++")

}
