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

    template <double (*F)(double, double, double, double, double), filter_type own_type>
    class filter_trigonometric: public filter {
    private:
        double* input = nullptr;
        double* factor_outer = nullptr;
        double* factor_inner = nullptr;
        double* phase = nullptr;
        double* offset = nullptr;
        double output = 0;
    public:
        filter_trigonometric() : filter() {}
        virtual ~filter_trigonometric() {}

        virtual void setup_filter(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const channel_mapping& input_channels, const std::string& own_id) override {
            MARK_UNUSED(initial_parameters);
            MARK_UNUSED(configuration);
            if(!input_channels.float_channels.contains("value_in")) {
                throw filter_config_exception("Unable to link input of trigonometric filter: channel mapping does not "
                                              "contain channel 'value_in' of type 'double'.", own_type, own_id);
            }
	        this->input = input_channels.float_channels.at("value_in");

            if(!input_channels.float_channels.contains("factor_outer")) {
                throw filter_config_exception("Unable to link input of trigonometric filter: channel mapping does not "
                                              "contain channel 'factor_outer' of type 'double'.", own_type, own_id);
            }
            this->factor_outer = input_channels.float_channels.at("factor_outer");

            if(!input_channels.float_channels.contains("factor_inner")) {
                throw filter_config_exception("Unable to link input of trigonometric filter: channel mapping does not "
                                              "contain channel 'factor_inner' of type 'double'.", own_type, own_id);
            }
            this->factor_inner = input_channels.float_channels.at("factor_inner");

            if(!input_channels.float_channels.contains("phase")) {
                throw filter_config_exception("Unable to link input of trigonometric filter: channel mapping does not "
                                              "contain channel 'phase' of type 'double'.", own_type, own_id);
            }
            this->phase = input_channels.float_channels.at("phase");

            if(!input_channels.float_channels.contains("offset")) {
                throw filter_config_exception("Unable to link input of trigonometric filter: channel mapping does not "
                                              "contain channel 'offset' of type 'double'.", own_type, own_id);
            }
            this->offset = input_channels.float_channels.at("offset");
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

    template <double (*F)(double, double, double, double, double, double), filter_type own_type>
    class filter_five_params: public filter {
    private:
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

        virtual void setup_filter(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const channel_mapping& input_channels, const std::string& own_id) override {
            MARK_UNUSED(initial_parameters);
            MARK_UNUSED(configuration);
            if(!input_channels.float_channels.contains("value_in")) {
                throw filter_config_exception("Unable to link input of trigonometric filter: channel mapping does not "
                                              "contain channel 'value_in' of type 'double'.", own_type, own_id);
            }
            this->input = input_channels.float_channels.at("value_in");

            if(!input_channels.float_channels.contains("factor_outer")) {
                throw filter_config_exception("Unable to link input of trigonometric filter: channel mapping does not "
                                              "contain channel 'factor_outer' of type 'double'.", own_type, own_id);
            }
            this->factor_outer = input_channels.float_channels.at("factor_outer");

            if(!input_channels.float_channels.contains("factor_inner")) {
                throw filter_config_exception("Unable to link input of trigonometric filter: channel mapping does not "
                                              "contain channel 'factor_inner' of type 'double'.", own_type, own_id);
            }
            this->factor_inner = input_channels.float_channels.at("factor_inner");

            if(!input_channels.float_channels.contains("phase")) {
                throw filter_config_exception("Unable to link input of trigonometric filter: channel mapping does not "
                                              "contain channel 'phase' of type 'double'.", own_type, own_id);
            }
            this->phase = input_channels.float_channels.at("phase");

            if(!input_channels.float_channels.contains("offset")) {
                throw filter_config_exception("Unable to link input of trigonometric filter: channel mapping does not "
                                              "contain channel 'offset' of type 'double'.", own_type, own_id);
            }
            this->offset = input_channels.float_channels.at("offset");

            if(!input_channels.float_channels.contains("length")) {
                throw filter_config_exception("Unable to link input of trigonometric filter: channel mapping does not "
                                              "contain channel 'length' of type 'double'.", own_type, own_id);
            }
            this->length = input_channels.float_channels.at("length");
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


    using filter_sine = filter_trigonometric<sin_deg, filter_type::filter_sine>;
    using filter_cosine = filter_trigonometric<cos_deg, filter_type::filter_cosine>;
    using filter_tangent = filter_trigonometric<tan_deg, filter_type::filter_tangent>;

    using filter_arcsine = filter_math_single<asin_deg, filter_type::filter_arcsine>;
    using filter_arccosine = filter_math_single<acos_deg, filter_type::filter_arccosine>;
    using filter_arctangent = filter_math_single<atan_deg, filter_type::filter_arctangent>;

    using filter_square = filter_five_params<square, filter_type::filter_square>;
    using filter_triangle = filter_trigonometric<triangle, filter_type::filter_triangle>;
    using filter_sawtooth = filter_trigonometric<sawtooth, filter_type::filter_sawtooth>;

    COMPILER_RESTORE("-Weffc++")
}
