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

        virtual void setup_filter(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const channel_mapping& input_channels) override {
            MARK_UNUSED(configuration);
            MARK_UNUSED(initial_parameters);
            MARK_UNUSED(input_channels);
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
//            auto a = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()-get_start_time());
//            auto b = a.count();
//            this->now = b;
        }

        virtual void scene_activated() override {}

    };

    template <typename T>
    class filter_delay: public filter {
    private:
        T* value = nullptr;
        std::chrono::duration<double> delay;
        std::chrono::time_point<std::chrono::system_clock> last_update;
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
                this->delay = std::chrono::duration<double>(std::stod(configuration.at("delay")));
            } catch (const std::invalid_argument& ex) {
                MARK_UNUSED(ex);
                throw filter_config_exception("Unable to setup delay filter: could not parse the 'delay'");
            }
            if constexpr (std::is_same<T, uint8_t>::value) {
                if(!input_channels.eight_bit_channels.contains("value")) {
                    throw filter_config_exception("Unable to link input of delay filter: channel mapping does not contain channel 'value' of type 'uint8_t'.");
                }
                this->input = input_channels.eight_bit_channels.at("value");
            } else if constexpr (std::is_same<T, uint16_t>::value) {
                if(!input_channels.sixteen_bit_channels.contains("value")) {
                    throw filter_config_exception("Unable to link input of delay filter: channel mapping does not contain channel 'value' of type 'uint16_t'.");
                }
                this->input = input_channels.sixteen_bit_channels.at("value");
            } else if constexpr (std::is_same<T, double>::value) {
                if(!input_channels.float_channels.contains("value")) {
                    throw filter_config_exception("Unable to link input of delay filter: channel mapping does not contain channel 'value' of type 'double'.");
                }
                this->input = input_channels.float_channels.at("value");
            } else {
                if(!input_channels.color_channels.contains("value")) {
                    throw filter_config_exception("Unable to link input of delay filter: channel mapping does not contain channel 'value' of type 'hsv_pixel'.");
                }
                this->input = input_channels.color_channels.at("value");
            }
        }

        virtual bool receive_update_from_gui(const std::string& key, const std::string& _value) override {
            MARK_UNUSED(key);
            MARK_UNUSED(_value);
            return false;
        }

        virtual void get_output_channels(channel_mapping& map, const std::string& name) override {
            map.float_channels[name + ":value"] = &output;
        }

        virtual void update() override {
            std::chrono::time_point<std::chrono::system_clock> now = std::chrono ::system_clock::now();
            bool timeout = now < (this->last_update + this->delay);
//            if constexpr (std::is_same<T, dmxfish::dmx::pixel>::value) {
//                this->value.hue = *value.hue * (this->last_update + this->delay < now);
//                this->value.saturation = *value.saturation * (this->last_update + this->delay < now);
//                this->value.iluminance = *value.iluminance * (this->last_update + this->delay < now);
//                this->last_update = std::chrono::system_clock::now() * (*value.iluminance <= 0) + (*value.iluminance > 0) * this->last_update;
//            } else {

                this->output = *value * !timeout + this->output * timeout;
                // does not work, because a timepoint is not scalable
                this->last_update = now * (*value <= 0 && !timeout) + this->last_update * (!(*value <= 0) || timeout);
                // is something possible like that? can the compiler make it more efficient (because of branch prediction)
                last_update = (*value <= 0 && !timeout)?now:last_update;
                // or should i use here also double as timestamp, or something like this answer: https://stackoverflow.com/questions/52202803/multiply-stdchrono-timepoint-by-a-scalar
//            }

        }

        virtual void scene_activated() override {}

    };

    using delay_switch_on_8bit = filter_delay<uint8_t>;
    using delay_switch_on_16bit = filter_delay<uint16_t>;
    using delay_switch_on_float = filter_delay<double>;
    using delay_switch_on_pixel = filter_delay<dmxfish::dmx::pixel>;

    COMPILER_RESTORE("-Weffc++")

}
