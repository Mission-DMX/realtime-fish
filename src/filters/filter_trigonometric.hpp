#pragma once

/*
 * The filters calculate trigonometric functions
 */

#include <cmath>

#include "filters/filter.hpp"
#include "lib/macros.hpp"
#include "filters/filter_math.hpp"


namespace dmxfish::filters {

    COMPILER_SUPRESS("-Weffc++")
    double sin_deg(double v, double f, double m, double p, double o){
        return f*std::sin(std::remainder((v+p)*m, 360.0)*M_PI/180) + o;
    }
    double cos_deg(double v, double f, double m, double p, double o){
        return f*std::cos(std::remainder((v+p)*m, 360.0)*M_PI/180) + o;
    }
    double tan_deg(double v, double f, double m, double p, double o){
        return f*std::tan(std::remainder((v+p)*m, 360.0)*M_PI/180) + o;
    }
    double triangle(double v, double f, double m, double p, double o){
        return f*(1.0-2.0*std::abs(std::remainder((v+p-90.0)*m, 360.0)/180.0)) + o;
    }
    double sawtooth(double v, double f, double m, double p, double o){
        return f*std::remainder((v+p)*m, 360.0)/180.0 + o;
    }
    double square(double v, double f, double m, double p, double o, double l){
        return f*(2.0*(std::remainder((v+p)*m, 360.0) > (l-180.0))-1) + o;
    }

    double asin_deg(double v){
        return std::asin(v)*180/M_PI;
    }
    double acos_deg(double v){
        return std::acos(v)*180/M_PI;
    }
    double atan_deg(double v){
        return std::atan(v)*180/M_PI;
    }

    template <double (*F)(double, double, double, double, double)>
    class filter_trigonometric: public filter {
    private:
        static constexpr double one = 1;
        static constexpr double zero = 0;
        double* input = nullptr;
        double* factor_outer = nullptr;
        double* factor_inner = nullptr;
        double* phase = nullptr;
        double* offset = nullptr;
        double output = 0;
    public:
        filter_trigonometric() : filter() {}
        virtual ~filter_trigonometric() {}

        virtual void setup_filter(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const channel_mapping& input_channels) override {
            MARK_UNUSED(initial_parameters);
            MARK_UNUSED(configuration);
            if(!input_channels.float_channels.contains("value")) {
                throw filter_config_exception("Unable to link input of trigonometric filter: channel mapping does not contain channel 'value' of type 'double'.");
            }
	        this->input = input_channels.float_channels.at("value");
            if(input_channels.float_channels.contains("factor_outer")) {
                this->factor_outer = input_channels.float_channels.at("factor_outer");
            } else {
                this->factor_outer = (double*) &this->one;
            }
            if(input_channels.float_channels.contains("factor_inner")) {
                this->factor_inner = input_channels.float_channels.at("factor_inner");
            } else {
                this->factor_inner = (double*) &this->one;
            }
            if(input_channels.float_channels.contains("phase")) {
                this->phase = input_channels.float_channels.at("phase");
            } else {
                this->phase = (double*) &this->zero;
            }
            if(input_channels.float_channels.contains("offset")) {
                this->offset = input_channels.float_channels.at("offset");
            } else {
                this->offset = (double*) &this->zero;
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
            this->output = F(*input, *factor_outer, *factor_inner, *phase, *offset);
        }

        virtual void scene_activated() override {}

    };

    template <double (*F)(double, double, double, double, double, double)>
    class filter_five_params: public filter {
    private:
        static constexpr double one = 1;
        static constexpr double zero = 0;
        static constexpr double half_circle = 180;
        double* input = nullptr;
        double* factor_outer = nullptr;
        double* factor_inner = nullptr;
        double* phase = nullptr;
        double* offset = nullptr;
        double* length = nullptr;
        double output = 0;
    public:
        filter_five_params() : filter() {}
        virtual ~filter_five_params() {}

        virtual void setup_filter(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const channel_mapping& input_channels) override {
            MARK_UNUSED(initial_parameters);
            MARK_UNUSED(configuration);
            if(!input_channels.float_channels.contains("value")) {
                throw filter_config_exception("Unable to link input of trigonometric filter: channel mapping does not contain channel 'value' of type 'double'.");
            }
            this->input = input_channels.float_channels.at("value");
            if(input_channels.float_channels.contains("factor_outer")) {
                this->factor_outer = input_channels.float_channels.at("factor_outer");
            } else {
                this->factor_outer = (double*) &this->one;
            }
            if(input_channels.float_channels.contains("factor_inner")) {
                this->factor_inner = input_channels.float_channels.at("factor_inner");
            } else {
                this->factor_inner = (double*) &this->one;
            }
            if(input_channels.float_channels.contains("phase")) {
                this->phase = input_channels.float_channels.at("phase");
            } else {
                this->phase = (double*) &this->zero;
            }
            if(input_channels.float_channels.contains("offset")) {
                this->offset = input_channels.float_channels.at("offset");
            } else {
                this->offset = (double*) &this->zero;
            }
            if(input_channels.float_channels.contains("length")) {
                this->length = input_channels.float_channels.at("length");
            } else {
                this->length = (double*) &this->half_circle;
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
            this->output = F(*input, *factor_outer, *factor_inner, *phase, *offset, *length);
        }

        virtual void scene_activated() override {}

    };


    using filter_sine = filter_trigonometric<sin_deg>;
    using filter_cosine = filter_trigonometric<cos_deg>;
    using filter_tangent = filter_trigonometric<tan_deg>;

    using filter_arcsine = filter_math_single<asin_deg>;
    using filter_arccosine = filter_math_single<acos_deg>;
    using filter_arctangent = filter_math_single<atan_deg>;

    using filter_square = filter_five_params<square>;
    using filter_triangle = filter_trigonometric<triangle>;
    using filter_sawtooth = filter_trigonometric<sawtooth>;

    COMPILER_RESTORE("-Weffc++")
}