#pragma once

/*
 * The filters calculate trigonometric functions
 */

#include <cmath>

#include "filters/filter.hpp"
#include "lib/macros.hpp"
#include "filters/filter_trigonometric.hpp"


namespace dmxfish::filters {

    COMPILER_SUPRESS("-Weffc++")

    using filter_logarithm = filter_trigonometric<std::log>;
    using filter_exponential = filter_trigonometric<std::exp>;

    template <typename T, T (*F)(T, T)>
    class filter_math: public filter {
    private:
        T* param1 = nullptr;
        T* param2 = nullptr;
        T output = 0;
    public:
        filter_math() : filter() {}
        virtual ~filter_math() {}

        virtual void setup_filter(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const channel_mapping& input_channels) override {
            MARK_UNUSED(initial_parameters);
            MARK_UNUSED(configuration);
            if(!input_channels.float_channels.contains("param1") || !input_channels.float_channels.contains("param2")) {
                throw filter_config_exception("Unable to link input of math filter: channel mapping does not contain channel 'param1' or 'param2' of type 'double'.");
            }
            this->param1 = input_channels.float_channels.at("param1");
            this->param2 = input_channels.float_channels.at("param2");
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
            this->output = F(*param1, *param2);
        }

        virtual void scene_activated() override {}

    };

    using filter_minimum = filter_math<double, std::fmin>;
    using filter_maximum = filter_math<double, std::fmax>;

    COMPILER_RESTORE("-Weffc++")

}
