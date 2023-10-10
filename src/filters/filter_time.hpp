#pragma once

/*
 * The filters calculate trigonometric functions
 */

#include <cmath>

#include "filters/filter.hpp"
#include "filters/util.hpp"
#include "lib/macros.hpp"
#include "lib/logging.hpp"

#include "global_vars.hpp"


namespace dmxfish::filters {

    COMPILER_SUPRESS("-Weffc++")

    class filter_time: public filter {
    private:
        double now = 0;
    public:
        filter_time() : filter() {}
        virtual ~filter_time() {}

        virtual void setup_filter(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const channel_mapping& input_channels) override {
            MARK_UNUSED(configuration);
            MARK_UNUSED(initial_parameters);
            MARK_UNUSED(input_channels);
            this->now = (double) (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()-get_start_time())).count();
        }

        virtual bool receive_update_from_gui(const std::string& key, const std::string& _value) override {
            MARK_UNUSED(key);
            MARK_UNUSED(_value);
            return false;
        }

        virtual void get_output_channels(channel_mapping& map, const std::string& name) override {
            map.float_channels[name + ":value"] = &now;
        }

        virtual void update() override {
            this->now = (double) (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()-get_start_time())).count();
        }

        virtual void scene_activated() override {}

    };

    template <typename T, bool(*F)(T)>
    class filter_delay: public filter {
    private:
        T* value = nullptr;
        double* time = nullptr;
        double delay;
        double last_update;
        T output = 0;
    public:
        filter_delay() : filter() {}
        virtual ~filter_delay() {}

        virtual void setup_filter(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const channel_mapping& input_channels) override {
            MARK_UNUSED(initial_parameters);
            if (!configuration.contains("delay")){
                throw filter_config_exception("Unable to setup delay filter: configuration does not contain a value for 'delay'");
            }
            try {
                this->delay = std::stod(configuration.at("delay"))*1000;
            } catch (const std::invalid_argument& ex) {
                MARK_UNUSED(ex);
                throw filter_config_exception("Unable to setup delay filter: could not parse the 'delay' as double");
            }
            if (input_channels.float_channels.contains("time")) {
                this->time = input_channels.float_channels.at("time");
            } else {
                this->time = &util::float_zero;
                ::spdlog::warn(std::string("delay filter had no time input, so it wont do anything"));
//                throw filter_config_exception("Unable to link input of delay filter: channel mapping does not contain channel 'time' of type 'double'. This input should come from the scenes global time node.");
            }
            if constexpr (std::is_same<T, uint8_t>::value) {
                this->value = input_channels.eight_bit_channels.contains("value_in") ? input_channels.eight_bit_channels.at("value_in") : &util::low_8bit;
            } else if constexpr (std::is_same<T, uint16_t>::value) {
                this->value = input_channels.sixteen_bit_channels.contains("value_in") ? input_channels.sixteen_bit_channels.at("value_in") : &util::low_16bit;
            } else if constexpr (std::is_same<T, double>::value) {
                this->value = input_channels.float_channels.contains("value_in") ? input_channels.float_channels.at("value_in") : &util::float_zero;
            } else {
                this->value = input_channels.color_channels.contains("value_in") ? input_channels.color_channels.at("value_in") : &util::color_white;
            }
        }

        virtual bool receive_update_from_gui(const std::string& key, const std::string& _value) override {
            MARK_UNUSED(key);
            MARK_UNUSED(_value);
            return false;
        }

        virtual void get_output_channels(channel_mapping& map, const std::string& name) override {
            if constexpr (std::is_same<T, uint8_t>::value) {
                map.eight_bit_channels[name + ":value"] = &output;
            } else if constexpr (std::is_same<T, uint16_t>::value) {
                map.sixteen_bit_channels[name + ":value"] = &output;
            } else if constexpr (std::is_same<T, double>::value) {
                map.float_channels[name + ":value"] = &output;
            } else {
                map.color_channels[name + ":value"] = &output;
            }
        }

        virtual void update() override {
            bool timeout = *time < (last_update + delay);
            if (F(*value)) {
                output = *value;
                last_update = *time;
            } else {
                if (!timeout){
                    output = *value;
                }
            }
        }

        virtual void scene_activated() override {
            if(F(0)){
                output = 0;
            }
            last_update = *time;
        }

    };

    using delay_switch_on_8bit = filter_delay<uint8_t, [](uint8_t a) { return a <= 0; }>;
    using delay_switch_on_16bit = filter_delay<uint16_t, [](uint16_t a) { return a <= 0; }>;
    using delay_switch_on_float = filter_delay<double, [](double a) { return a <= 0.0; }>;

    using delay_switch_off_8bit = filter_delay<uint8_t, [](uint8_t a) { return (a > 0); }>;
    using delay_switch_off_16bit = filter_delay<uint16_t, [](uint16_t a) { return (a > 0); }>;
    using delay_switch_off_float = filter_delay<double, [](double a) { return (a > 0.0); }>;

    COMPILER_RESTORE("-Weffc++")

}
