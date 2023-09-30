#include "filters/util.hpp"

//#include <cmath>

//
//#include "lib/macros.hpp"
//#include "lib/logging.hpp"
//#include "dmx/pixel.hpp"


namespace dmxfish::filters {
    int util::count_occurence_of(const std::string &base_string, std::string pattern, size_t start, size_t end) {
        int occurrences = 0;
        while ((start = base_string.find(pattern, start)) != std::string::npos && start <= end) {
            ++occurrences;
            start += pattern.length();
        }
        return occurrences;
    }
}
