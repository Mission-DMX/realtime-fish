// Created by LRahlff on 02.05.23.
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

/*
 * The filters calculate trigonometric functions
 */

#include <cmath>

#include "filters/filter.hpp"
#include "lib/macros.hpp"

#include "global_vars.hpp"


namespace dmxfish::filters {

    COMPILER_SUPRESS("-Weffc++")

    class filter_time: public filter {
    private:
        double now = 0;
    public:
        filter_time() : filter() {}
        virtual ~filter_time() {}

        virtual void setup_filter(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const channel_mapping& input_channels, const std::string& own_id) override {
            MARK_UNUSED(configuration);
            MARK_UNUSED(initial_parameters);
            MARK_UNUSED(input_channels);
            MARK_UNUSED(own_id);
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

        virtual void scene_activated() override {
            // As there are filters requiring the the current time as soon as the scene gets activated,
            // we need to perform the update here.
            this->update();
        }

    };

    template <typename T, bool(*F)(T), filter_type own_type>
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

        virtual void setup_filter(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const channel_mapping& input_channels, const std::string& own_id) override {
            MARK_UNUSED(initial_parameters);
            if (!configuration.contains("delay")){
                throw filter_config_exception("Unable to setup delay filter: configuration does not contain a value for"
                                              " 'delay'", own_type, own_id);
            }
            try {
                this->delay = std::stod(configuration.at("delay"))*1000;
            } catch (const std::invalid_argument& ex) {
                MARK_UNUSED(ex);
                throw filter_config_exception("Unable to setup delay filter: could not parse the 'delay' as double",
                                              own_type, own_id);
            }
            if(!input_channels.float_channels.contains("time")) {
                throw filter_config_exception("Unable to link input of delay filter: channel mapping does not contain "
                                              "channel 'time' of type 'double'. This input should come from the scenes "
                                              "global time node.", own_type, own_id);
            }
            this->time = input_channels.float_channels.at("time");
            if constexpr (std::is_same<T, uint8_t>::value) {
                if(!input_channels.eight_bit_channels.contains("value_in")) {
                    throw filter_config_exception("Unable to link input of delay filter: channel mapping does not "
                                                  "contain channel 'value_in' of type 'uint8_t'.", own_type, own_id);
                }
                this->value = input_channels.eight_bit_channels.at("value_in");
            } else if constexpr (std::is_same<T, uint16_t>::value) {
                if(!input_channels.sixteen_bit_channels.contains("value_in")) {
                    throw filter_config_exception("Unable to link input of delay filter: channel mapping does not "
                                                  "contain channel 'value_in' of type 'uint16_t'.", own_type, own_id);
                }
                this->value = input_channels.sixteen_bit_channels.at("value_in");
            } else if constexpr (std::is_same<T, double>::value) {
                if(!input_channels.float_channels.contains("value_in")) {
                    throw filter_config_exception("Unable to link input of delay filter: channel mapping does not "
                                                  "contain channel 'value_in' of type 'double'.", own_type, own_id);
                }
                this->value = input_channels.float_channels.at("value_in");
            } else {
                if(!input_channels.color_channels.contains("value_in")) {
                    throw filter_config_exception("Unable to link input of delay filter: channel mapping does not "
                                                  "contain channel 'value_in' of type 'hsv_pixel'.", own_type, own_id);
                }
                this->value = input_channels.color_channels.at("value_in");
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

    using delay_switch_on_8bit = filter_delay<uint8_t, [](uint8_t a) { return a <= 0; }, filter_type::delay_switch_on_8bit>;
    using delay_switch_on_16bit = filter_delay<uint16_t, [](uint16_t a) { return a <= 0; }, filter_type::delay_switch_on_16bit>;
    using delay_switch_on_float = filter_delay<double, [](double a) { return a <= 0.0; }, filter_type::delay_switch_on_float>;

    using delay_switch_off_8bit = filter_delay<uint8_t, [](uint8_t a) { return (a > 0); }, filter_type::delay_switch_off_8bit>;
    using delay_switch_off_16bit = filter_delay<uint16_t, [](uint16_t a) { return (a > 0); }, filter_type::delay_switch_off_16bit>;
    using delay_switch_off_float = filter_delay<double, [](double a) { return (a > 0.0); }, filter_type::delay_switch_off_float>;

    COMPILER_RESTORE("-Weffc++")

}
