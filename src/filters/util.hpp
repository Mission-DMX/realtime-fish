#pragma once

/*
 * The filters hold a cue sequence
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
        enum channel_t{
            EIGHT_BIT,
            SIXTEEN_BIT,
            FLOAT,
            COLOR
        };
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
