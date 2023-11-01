#pragma once

/*
 * The filters calculate basic math functions
 */

#include <cmath>

#include "filters/filter.hpp"
#include "lib/macros.hpp"


namespace dmxfish::filters {

    COMPILER_SUPRESS("-Weffc++")
    template <double (*F)(double), filter_type own_type>
    class filter_math_single: public filter {
    private:
        double* input = nullptr;
        double output = 0;
    public:
        filter_math_single() : filter() {}
        virtual ~filter_math_single() {}

        virtual void setup_filter(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const channel_mapping& input_channels, const std::string& own_id) override {
            MARK_UNUSED(initial_parameters);
            MARK_UNUSED(configuration);
            if(!input_channels.float_channels.contains("value_in")) {
                throw filter_config_exception("Unable to link input of math filter: channel mapping does not contain "
                                              "channel 'value_in' of type 'double'.", own_type, own_id);
            }
            this->input = input_channels.float_channels.at("value_in");
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
            this->output = (*input);
        }

        virtual void scene_activated() override {}

    };

    template <typename T, T (*F)(T, T), filter_type own_type>
    class filter_math_dual: public filter {
    private:
        T* param1 = nullptr;
        T* param2 = nullptr;
        T output = 0;
    public:
        filter_math_dual() : filter() {}
        virtual ~filter_math_dual() {}

        virtual void setup_filter(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const channel_mapping& input_channels, const std::string& own_id) override {
            MARK_UNUSED(initial_parameters);
            MARK_UNUSED(configuration);
            if(!input_channels.float_channels.contains("param1") || !input_channels.float_channels.contains("param2")) {
                throw filter_config_exception("Unable to link input of math filter: channel mapping does not contain "
                                              "channel 'param1' or 'param2' of type 'double'.", own_type, own_id);
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


    using filter_logarithm = filter_math_single<std::log, filter_type::filter_logarithm>;
    using filter_exponential = filter_math_single<std::exp, filter_type::filter_exponential>;

    using filter_minimum = filter_math_dual<double, std::fmin, filter_type::filter_minimum>;
    using filter_maximum = filter_math_dual<double, std::fmax, filter_type::filter_maximum>;

    COMPILER_RESTORE("-Weffc++")

}
