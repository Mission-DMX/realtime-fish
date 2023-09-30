#pragma once

/*
 * The filters hold a cue sequence
 */

//#include <vector>
//#include <functional>
//#include "filters/filter.hpp"
#include <string>

namespace dmxfish::filters {
//    COMPILER_SUPRESS("-Weffc++")


    class util{
    public:
        static int count_occurence_of(const std::string &base_string, std::string pattern, size_t start, size_t end);

    };

//    COMPILER_RESTORE("-Weffc++")

}
