#pragma once

/*
 * The filters calculate trigonometric functions
 */

#include <cmath>

#include "filters/filter.hpp"
#include "lib/macros.hpp"


namespace dmxfish::filters {

    double sin_deg(double v){
        return std::sin(v*M_PI/180);
    }
    double cos_deg(double v){
        return std::cos(v*M_PI/180);
    }
    double tan_deg(double v){
        return std::tan(v*M_PI/180);
    }

    double asin_deg(double v){
        return std::asin(v*180/M_PI);
    }
    double acos_deg(double v){
        return std::acos(v*180/M_PI);
    }
    double atan_deg(double v){
        return std::atan(v*180/M_PI);
    }

    template <double (*F)(double)>
    class filter_trigonometric: public filter {
    private:
        double* input = nullptr;
        double output = 0;
    public:
        filter_trigonometric() : filter() {}
        virtual ~filter_trigonometric() {}

        virtual void setup_filter(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const channel_mapping& input_channels) override {
            MARK_UNUSED(initial_parameters);
            MARK_UNUSED(configuration);
	    if(!input_channels.float_channels.contains("value")) {
		    throw filter_config_exception("Unable to link input of trigonometric_sinus filter: channel mapping does not contain channel 'value' of type 'double'.");
	    }
	        this->input = input_channels.float_channels.at("value");
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

    using filter_trigonometric_sine = filter_trigonometric<sin_deg>;
    using filter_trigonometric_cosine = filter_trigonometric<cos_deg>;
    using filter_trigonometric_tangent = filter_trigonometric<tan_deg>;
    using filter_trigonometric_arcsine = filter_trigonometric<asin_deg>;
    using filter_trigonometric_arccosine = filter_trigonometric<acos_deg>;
    using filter_trigonometric_arctangent = filter_trigonometric<atan_deg>;
}
