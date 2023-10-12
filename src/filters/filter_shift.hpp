#pragma once

/*
 * This filter shifts the input in through several outputs
 */
#include "filters/filter.hpp"
#include "lib/macros.hpp"

#include "filters/util.hpp"
#include "lib/logging.hpp"

namespace dmxfish::filters {

    COMPILER_SUPRESS("-Weffc++")

    template <typename T>
    class filter_shift: public filter {
    private:
        T* input = nullptr;
        double* time = nullptr;
        double* switch_time = nullptr;
        std::vector<T> values;
        double last_update;
    public:
        filter_shift() : filter() {}
        virtual ~filter_shift() {}

        virtual void setup_filter(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const channel_mapping& input_channels) override {
            MARK_UNUSED(initial_parameters);
            if (input_channels.float_channels.contains("time")) {
                this->time = input_channels.float_channels.at("time");
            } else {
                this->time = &util::float_zero;
                ::spdlog::warn(std::string("shift filter had no time input, so it wont shift"));
//                throw filter_config_exception("Unable to link input of shift filter: channel mapping does not contain channel 'time' of type 'double'. This input should come from the scenes global time node.");
            }
            this->switch_time = input_channels.float_channels.contains("switch_time") ? input_channels.float_channels.at("switch_time") : &util::float_1000;
            
            if (!configuration.contains("nr_outputs")){
                throw filter_config_exception("Unable to setup shift filter: configuration does not contain a value for 'nr_outputs'");
            }
            int nr_outputs = 0;
            try {
                nr_outputs = std::stoi(configuration.at("nr_outputs"));
            } catch (const std::invalid_argument& ex) {
                MARK_UNUSED(ex);
                throw filter_config_exception("Unable to setup shift filter: could not parse the 'nr_outputs' as int");
            }
            if (nr_outputs < 1) {
                throw filter_config_exception("Unable to setup shift filter: the number of outputs is less then 1");
            }
            
            
            if constexpr (std::is_same<T, uint8_t>::value) {
                this->input = input_channels.eight_bit_channels.contains("input") ? input_channels.eight_bit_channels.at("input") : &util::low_8bit;
                values = std::vector(nr_outputs, (uint8_t) 0);
            } else if constexpr (std::is_same<T, uint16_t>::value) {
                this->input = input_channels.sixteen_bit_channels.contains("input") ? input_channels.sixteen_bit_channels.at("input") : &util::low_16bit;
                values = std::vector(nr_outputs, (uint16_t) 0);
            } else if constexpr (std::is_same<T, double>::value) {
                this->input = input_channels.float_channels.contains("input") ? input_channels.float_channels.at("input") : &util::float_zero;
                values = std::vector(nr_outputs, 0.0);
            } else {
                this->input = input_channels.color_channels.contains("input") ? input_channels.color_channels.at("input") : &util::color_white;
                values = std::vector(nr_outputs, dmxfish::dmx::pixel());
            }
        }

        virtual bool receive_update_from_gui(const std::string& key, const std::string& _value) override {
            MARK_UNUSED(key);
            MARK_UNUSED(_value);
            return false;
        }

        virtual void get_output_channels(channel_mapping& map, const std::string& name) override {
            if constexpr (std::is_same<T, uint8_t>::value) {
                for(size_t i = 0; i < values.size(); i++) {
                    map.eight_bit_channels[name + ":output_" + std::to_string(i+1)] = &values.at(i);
                }
            } else if constexpr (std::is_same<T, uint16_t>::value) {
                for(size_t i = 0; i < values.size(); i++) {
                    map.sixteen_bit_channels[name + ":output_" + std::to_string(i+1)] = &values.at(i);
                }
            } else if constexpr (std::is_same<T, double>::value) {
                for(size_t i = 0; i < values.size(); i++) {
                    map.float_channels[name + ":output_" + std::to_string(i+1)] = &values.at(i);
                }
            } else {
                for(size_t i = 0; i < values.size(); i++) {
                    map.color_channels[name + ":output_" + std::to_string(i+1)] = &values.at(i);
                }
            }
        }

        virtual void update() override {
            if (last_update + *switch_time <= *time){
                for(size_t i = values.size() - 1; i > 0; i--){
                    values.at(i) = values.at(i-1);
                }
                values.at(0) = *input;
                last_update = last_update + *switch_time;
            }
        }

        virtual void scene_activated() override {}

    };

    using filter_shift_8bit = filter_shift<uint8_t>;
    using filter_shift_16bit = filter_shift<uint16_t>;
    using filter_shift_float = filter_shift<double>;
    using filter_shift_color = filter_shift<dmxfish::dmx::pixel>;

    COMPILER_RESTORE("-Weffc++")

}
