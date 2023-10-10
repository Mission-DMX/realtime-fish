#include "filters/util.hpp"

//#include <cmath>

//
//#include "lib/macros.hpp"
//#include "lib/logging.hpp"


namespace dmxfish::filters {

    double util::float_zero = 0.0;
    double util::float_one = 1.0;
    double util::float_180 = 180.0;
    double util::float_1000 = 1000.0;
    uint8_t util::low_8bit = 0;
    uint8_t util::high_8bit = 255;
    uint16_t util::low_16bit = 0;
    uint16_t util::high_16bit = 65535;
    dmxfish::dmx::pixel util::color_white = dmxfish::dmx::pixel(0,0,1);

    int util::count_occurence_of(const std::string &base_string, std::string pattern, size_t start, size_t end) {
        int occurrences = 0;
        while ((start = base_string.find(pattern, start)) != std::string::npos && start <= end) {
            ++occurrences;
            start += pattern.length();
        }
        return occurrences;
    }

    void util::init_mapping(
            const std::string &mappingstr,
            std::function<void(int)> reserve_space_8bit,
            std::function<void(int)> reserve_space_16bit,
            std::function<void(int)> reserve_space_float,
            std::function<void(int)> reserve_space_color,
            std::function<void(std::string&)> init_values_8bit,
            std::function<void(std::string&)> init_values_16bit,
            std::function<void(std::string&)> init_values_float,
            std::function<void(std::string&)> init_values_color
    ){
        if (mappingstr.length() < 1){
            return;
        }
        reserve_space_8bit(util::count_occurence_of(mappingstr, ":8bit", 0, mappingstr.size()));
        reserve_space_16bit(util::count_occurence_of(mappingstr, ":16bit", 0, mappingstr.size()));
        reserve_space_float(util::count_occurence_of(mappingstr, ":float", 0, mappingstr.size()));
        reserve_space_color(util::count_occurence_of(mappingstr, ":color", 0, mappingstr.size()));

        size_t start_pos = 0;
        auto next_pos = mappingstr.find(";");
        while (true) {
            const auto sign = mappingstr.find(":", start_pos);
            std::string channel_type = mappingstr.substr(sign + 1, next_pos - sign - 1);
            std::string channel_name = mappingstr.substr(start_pos, sign - start_pos);
            if (!channel_type.compare("8bit")) {
                init_values_8bit(channel_name);
            } else if (!channel_type.compare("16bit")) {
                init_values_16bit(channel_name);
            } else if (!channel_type.compare("float")) {
                init_values_float(channel_name);
            } else if (!channel_type.compare("color")) {
                init_values_color(channel_name);
            } else {
                throw filter_config_exception(std::string("can not recognise channel type: ") +
                                              mappingstr.substr(sign + 1, next_pos - sign - 1));
            }
            if (next_pos >= mappingstr.length()) {
                break;
            }
            start_pos = next_pos + 1;
            next_pos = mappingstr.find(";", start_pos);
        }
    }
    

}
