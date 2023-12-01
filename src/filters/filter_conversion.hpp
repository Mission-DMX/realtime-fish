#pragma once

/*
 * The filters defined in this file provide adapters to other formats.
 */

#include <cmath>

#include "filters/filter.hpp"
#include "lib/macros.hpp"
#include "lib/logging.hpp"
#include <cfloat>

namespace dmxfish::filters {
COMPILER_SUPRESS("-Weffc++")
    class filter_16bit_to_dual_byte : public filter {
    private:
        uint16_t* input = nullptr;
        uint8_t lower_output = 0;
        uint8_t upper_output = 0;
    public:
        filter_16bit_to_dual_byte() : filter() {}
        virtual ~filter_16bit_to_dual_byte() {}

        virtual void setup_filter(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const channel_mapping& input_channels, const std::string& own_id) override {
            MARK_UNUSED(initial_parameters);
            MARK_UNUSED(configuration);
            if(!input_channels.sixteen_bit_channels.contains("value")) {
                throw filter_config_exception("Unable to link input of 16 bit splitting filter: channel mapping does "
                                              "not contain channel 'value' of type 'uint16_t'.",
                                              filter_type::filter_16bit_to_dual_byte, own_id);
            }
            this->input = input_channels.sixteen_bit_channels.at("value");
        }

        virtual bool receive_update_from_gui(const std::string& key, const std::string& _value) override {
            MARK_UNUSED(key);
            MARK_UNUSED(_value);
            return false;
        }

        virtual void get_output_channels(channel_mapping& map, const std::string& name) override {
            map.eight_bit_channels[name + ":value_lower"] = &lower_output;
            map.eight_bit_channels[name + ":value_upper"] = &upper_output;
        }

        virtual void update() override {
            this->lower_output = (uint8_t) (*input & (0x00FF));
            this->upper_output = (uint8_t) ((*input & (0xFF00)) >> 8);
	    }

        virtual void scene_activated() override {}

    };

    class filter_16bit_to_bool : public filter {
    private:
        uint16_t* input = nullptr;
        uint8_t output = 0;
    public:
        filter_16bit_to_bool() : filter() {}
        virtual ~filter_16bit_to_bool() {}

        virtual void setup_filter(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const channel_mapping& input_channels, const std::string& own_id) override {
            MARK_UNUSED(initial_parameters);
            MARK_UNUSED(configuration);
            if(!input_channels.sixteen_bit_channels.contains("value_in")) {
                throw filter_config_exception("Unable to link input of bool conversion filter: channel mapping does not"
                                              " contain channel 'value_in' of type 'uint16_t'.",
                                              filter_type::filter_16bit_to_bool, own_id);
            }
            this->input = input_channels.sixteen_bit_channels.at("value_in");
        }

        virtual bool receive_update_from_gui(const std::string& key, const std::string& _value) override {
            MARK_UNUSED(key);
            MARK_UNUSED(_value);
            return false;
        }

        virtual void get_output_channels(channel_mapping& map, const std::string& name) override {
            map.eight_bit_channels[name + ":value"] = &output;
        }

        virtual void update() override {
            this->output = *input > 0 ? 1 : 0;
        }

        virtual void scene_activated() override {}

    };

    class filter_multiply_add : public filter {
    private:
        double* input_factor_1 = nullptr;
	double* input_factor_2 = nullptr;
	double* input_summand = nullptr;
	double output = 0;
    public:
        filter_multiply_add() : filter() {}
        virtual ~filter_multiply_add() {}

        virtual void setup_filter(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const channel_mapping& input_channels, const std::string& own_id) override {
            MARK_UNUSED(initial_parameters);
            MARK_UNUSED(configuration);
            if(!input_channels.float_channels.contains("factor1") ||
                    !input_channels.float_channels.contains("factor2") ||
                    !input_channels.float_channels.contains("summand")) {
                throw filter_config_exception("Unable to link input of multiply add filter: channel mapping does not "
                                              "contain required channels.",
                                              filter_type::filter_multiply_add,
                                              own_id);
            }
            this->input_factor_1 = input_channels.float_channels.at("factor1");
            this->input_factor_2 = input_channels.float_channels.at("factor2");
            this->input_summand = input_channels.float_channels.at("summand");
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
            this->output = (*input_factor_1 * *input_factor_2) + *input_summand;
        }

        virtual void scene_activated() override {}

    };

    template <typename T, filter_type own_type>
    class filter_float_to_X_template : public filter {
    private:
        double* input = nullptr;
        T output = 0;
    public:
        filter_float_to_X_template() : filter() {
            static_assert(std::is_same<T, uint8_t>::value || std::is_same<T, uint16_t>::value || std::is_same<T, double>::value, "unsupported data format.");
        }
        virtual ~filter_float_to_X_template() {}

        virtual void setup_filter(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const channel_mapping& input_channels, const std::string& own_id) override {
            MARK_UNUSED(initial_parameters);
            MARK_UNUSED(configuration);
            if(!input_channels.float_channels.contains("value_in")) {
                if constexpr (std::is_same<T, uint8_t>::value) {
                    throw filter_config_exception("Unable to link input of float to 16 bit conversion filter: channel "
                                                  "mapping does not contain channel 'value_in' of type 'double'.",
                                                  own_type, own_id);
                } else if constexpr (std::is_same<T, uint16_t>::value) {
                    throw filter_config_exception("Unable to link input of float to 8 bit conversion filter: channel "
                                                  "mapping does not contain channel 'value_in' of type 'double'.",
                                                  own_type, own_id);
                } else {
                    throw filter_config_exception("Unable to link input of number rounding filter: channel mapping "
                                                  "does not contain channel 'value_in' of type 'double'.",
                                                  own_type, own_id);
                }
            }
            this->input = input_channels.float_channels.at("value_in");
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
            } else {
                map.float_channels[name + ":value"] = &output;
            }
        }

        virtual void update() override {
            this->output = (T) std::round(*input);
        }

        virtual void scene_activated() override {}

    };

    using filter_float_to_16bit = filter_float_to_X_template<uint16_t, filter_type::filter_float_to_16bit>;
    using filter_float_to_8bit = filter_float_to_X_template<uint8_t, filter_type::filter_float_to_8bit>;
    using filter_round_number = filter_float_to_X_template<double, filter_type::filter_round_number>;

    class filter_pixel_to_rgb_channels : public filter {
    private:
        dmxfish::dmx::pixel* input = nullptr;
        uint8_t r = 0, g = 0, b = 0;
    public:
        filter_pixel_to_rgb_channels() : filter() {}
        virtual ~filter_pixel_to_rgb_channels() {}

        virtual void setup_filter(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const channel_mapping& input_channels, const std::string& own_id) override {
            MARK_UNUSED(initial_parameters);
            MARK_UNUSED(configuration);
            if(!input_channels.color_channels.contains("value")) {
                throw filter_config_exception("Unable to link input of color to rgb filter: channel mapping does not "
                                              "contain channel 'value' of type 'color'.",
                                              filter_type::filter_pixel_to_rgb_channels, own_id);
            }
            this->input = input_channels.color_channels.at("value");
        }

        virtual bool receive_update_from_gui(const std::string& key, const std::string& value) override {
            MARK_UNUSED(key);
            MARK_UNUSED(value);
            return false;
        }

        virtual void get_output_channels(channel_mapping& map, const std::string& name) override {
            map.eight_bit_channels[name + ":r"] = &r;
            map.eight_bit_channels[name + ":g"] = &g;
            map.eight_bit_channels[name + ":b"] = &b;
        }

        virtual void update() override {
            input->pixel_to_rgb(r, g, b);
        }

        virtual void scene_activated() override {}
    };

    class filter_pixel_to_rgbw_channels : public filter {
    private:
        dmxfish::dmx::pixel* input = nullptr;
        uint8_t r = 0, g = 0, b = 0, w = 0;
    public:
        filter_pixel_to_rgbw_channels() : filter() {}
        virtual ~filter_pixel_to_rgbw_channels() {}

        virtual void setup_filter(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const channel_mapping& input_channels, const std::string& own_id) override {
            MARK_UNUSED(initial_parameters);
            MARK_UNUSED(configuration);
            if(!input_channels.color_channels.contains("value")) {
                throw filter_config_exception("Unable to link input of color to rgbw filter: channel mapping does not "
                                              "contain channel 'value' of type 'color'.",
                                              filter_type::filter_pixel_to_rgbw_channels, own_id);
            }
            this->input = input_channels.color_channels.at("value");
        }

        virtual bool receive_update_from_gui(const std::string& key, const std::string& value) override {
            MARK_UNUSED(key);
            MARK_UNUSED(value);
            return false;
        }

        virtual void get_output_channels(channel_mapping& map, const std::string& name) override {
            map.eight_bit_channels[name + ":r"] = &r;
            map.eight_bit_channels[name + ":g"] = &g;
            map.eight_bit_channels[name + ":b"] = &b;
            map.eight_bit_channels[name + ":w"] = &w;
        }

        virtual void update() override {
            input->pixel_to_rgbw(r, g, b, w);
        }

        virtual void scene_activated() override {}
    };

    class filter_floats_to_pixel : public filter {
    private:
        dmxfish::dmx::pixel output;
        double* h = nullptr;
        double* s = nullptr;
        double* i = nullptr;
    public:
        filter_floats_to_pixel(): filter(), output{} {}
        virtual ~filter_floats_to_pixel() {}

        virtual void setup_filter(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const channel_mapping& input_channels, const std::string& own_id) override {
            MARK_UNUSED(initial_parameters);
            MARK_UNUSED(configuration);
            if(!input_channels.float_channels.contains("h") || !input_channels.float_channels.contains("s") || !input_channels.float_channels.contains("i")) {
                throw filter_config_exception("Unable to link input of float to color filter: channel mapping does not "
                                              "contain input channels.",
                                              filter_type::filter_floats_to_pixel, own_id);
            }
            this->h = input_channels.float_channels.at("h");
            this->s = input_channels.float_channels.at("s");
            this->i = input_channels.float_channels.at("i");
        }

        virtual bool receive_update_from_gui(const std::string& key, const std::string& value) override {
            MARK_UNUSED(key);
            MARK_UNUSED(value);
            return false;
        }

        virtual void get_output_channels(channel_mapping& map, const std::string& name) override {
            map.color_channels[name + ":value"] = &output;
        }

        virtual void update() override {
            this->output = dmxfish::dmx::pixel(*this->h, *this->s, *this->i);
        }

        virtual void scene_activated() override {}
    };

    class filter_pixel_to_floats : public filter {
    private:
        dmxfish::dmx::pixel* input = nullptr;
        double h = 0;
        double s = 0;
        double i = 0;
    public:
        filter_pixel_to_floats(): filter() {}
        virtual ~filter_pixel_to_floats() {}

        virtual void setup_filter(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const channel_mapping& input_channels, const std::string& own_id) override {
            MARK_UNUSED(initial_parameters);
            MARK_UNUSED(configuration);
            if(!input_channels.color_channels.contains("input")) {
                throw filter_config_exception("Unable to link input of color to float filter: channel mapping does not "
                                              "contain the input channel.",
                                              filter_type::filter_pixel_to_floats, own_id);
            }
            this->input = input_channels.color_channels.at("input");
        }

        virtual bool receive_update_from_gui(const std::string& key, const std::string& value) override {
            MARK_UNUSED(key);
            MARK_UNUSED(value);
            return false;
        }

        virtual void get_output_channels(channel_mapping& map, const std::string& name) override {
            map.float_channels[name + ":hue"] = &h;
            map.float_channels[name + ":saturation"] = &s;
            map.float_channels[name + ":iluminance"] = &i;
        }

        virtual void update() override {
            this->h = this->input->hue;
            this->s = this->input->saturation;
            this->i = this->input->iluminance;
        }

        virtual void scene_activated() override {}
    };

    template <typename T, filter_type own_type>
    class filter_to_float_template : public filter {
    private:
        T* input = nullptr;
        double output = 0.0;
    public:
        filter_to_float_template() : filter() {}
        virtual ~filter_to_float_template() {}

        virtual void setup_filter(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const channel_mapping& input_channels, const std::string& own_id) override {
            MARK_UNUSED(initial_parameters);
            MARK_UNUSED(configuration);

            if constexpr (std::is_same<T, uint8_t>::value) {
                if(!input_channels.eight_bit_channels.contains("value_in")) {
                    throw filter_config_exception("Unable to link input of float conversion filter: channel mapping "
                                                  "does not contain channel 'value_in' of type 'uint8_t'.",
                                                  own_type, own_id);
                }
                this->input = input_channels.eight_bit_channels.at("value_in");
            } else if constexpr (std::is_same<T, uint16_t>::value) {
                if(!input_channels.sixteen_bit_channels.contains("value_in")) {
                    throw filter_config_exception("Unable to link input of float conversion filter: channel mapping "
                                                  "does not contain channel 'value_in' of type 'uint16_t'.",
                                                  own_type, own_id);
                }
                this->input =input_channels.sixteen_bit_channels.at("value_in");
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
            this->output = (double) *input;
        }

        virtual void scene_activated() override {}

    };

    using filter_8bit_to_float = filter_to_float_template<uint8_t, filter_type::filter_8bit_to_float>;
    using filter_16bit_to_float = filter_to_float_template<uint16_t, filter_type::filter_16bit_to_float>;

    template <typename T, filter_type own_type>
    class float_map_range : public filter {
    private:
        double* input = nullptr;
        double lower_bound_in = 0.0;
        double upper_bound_in = 1.0;
        double lower_bound_out = 0.0;
        double upper_bound_out = 1.0;
        bool limit = false;
        T output = 0;

        inline constexpr void setdefaultboundvalue(){
            if constexpr (std::is_same<T, uint8_t>::value) {
                this->upper_bound_out = 255.0;
            } else if constexpr (std::is_same<T, uint16_t>::value) {
                this->upper_bound_out = 65535.0;
            } else {
                this->upper_bound_out = 1.0;
            }
        }

    public:
        float_map_range() : filter() {}
        virtual ~float_map_range() {}

        virtual void setup_filter(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const channel_mapping& input_channels, const std::string& own_id) override {
            MARK_UNUSED(initial_parameters);
//            MARK_UNUSED(configuration);
            if(!input_channels.float_channels.contains("value_in")) {
                throw filter_config_exception("Unable to link input of float map range filter conversion filter: channel mapping "
                                              "does not contain channel 'value_in' of type 'float'.",
                                              own_type, own_id);
            }
            this->input = input_channels.float_channels.at("value_in");

            setdefaultboundvalue();

            if(configuration.contains("limit_range")) {
                if (!(std::string("") == configuration.at("limit_range"))) {
                    if(!receive_update_from_gui("limit_range", configuration.at("limit_range"))){
                        throw filter_config_exception(std::string("Unable to set the parameter for limiting the tange to " + configuration.at("limit_range")), own_type, own_id);
                    }
                }
            }

            if(configuration.contains("lower_bound_in")) {
                if (!(std::string("") == configuration.at("lower_bound_in"))) {
                    if(!receive_update_from_gui("lower_bound_in", configuration.at("lower_bound_in"))){
                        throw filter_config_exception(std::string("Unable to parse lower_bound_in of float map range filter conversion filter "), own_type, own_id);
                    }
                }
            }

            if(configuration.contains("upper_bound_in")) {
                if (!(std::string("") == configuration.at("upper_bound_in"))) {
                    if(!receive_update_from_gui("upper_bound_in", configuration.at("upper_bound_in"))){
                        throw filter_config_exception(std::string("Unable to parse upper_bound_in of float map range filter conversion filter "), own_type, own_id);
                    }
                }
            }

            if(configuration.contains("lower_bound_out")) {
                if (!(std::string("") == configuration.at("lower_bound_out"))) {
                    if(!receive_update_from_gui("lower_bound_out", configuration.at("lower_bound_out"))){
                        throw filter_config_exception(std::string("Unable to parse lower_bound_out of float map range filter conversion filter "), own_type, own_id);
                    }
                }
            }

            if(configuration.contains("upper_bound_out")) {
                if (!(std::string("") == configuration.at("upper_bound_out"))) {
                    if(!receive_update_from_gui("upper_bound_out", configuration.at("upper_bound_out"))){
                        throw filter_config_exception(std::string("Unable to parse upper_bound_out of float map range filter conversion filter "), own_type, own_id);
                    }
                }
            }
        }

        virtual bool receive_update_from_gui(const std::string& key, const std::string& _value) override {
            if("lower_bound_in" == key) {
                try {
                    double val = std::stod(_value);
//                    if (this->upper_bound_in == val) {
                    if (std::abs(val - this->lower_bound_in) < DBL_MIN) {
                        ::spdlog::info("could not set the lower_bound_in to the value of the upper_bound_in");
                        return false;
                    }
                    this->lower_bound_in = val;
                    return true;
                } catch (const std::invalid_argument &ex) {
                    ::spdlog::info("could not parse in filter float_map_range the value {} as float, lower_bound_in.", _value);
                    return false;
                } catch (const std::out_of_range &ex) {
                    ::spdlog::info("could not parse in filter float_map_range the value {} as float, lower_bound_in.", _value);
                    return false;
                }
            }

            if("upper_bound_in" == key) {
                try {
                    double val = std::stod(_value);
//                    if (val == this->lower_bound_in) {
                    if (std::abs(val - this->lower_bound_in) < DBL_MIN) {
                        ::spdlog::info("could not set the upper_bound_in to the value of the lower_bound_in");
                        return false;
                    }
                    this->upper_bound_in = val;
                    return true;
                } catch (const std::invalid_argument &ex) {
                    ::spdlog::info("could not parse in filter float_map_range the value {} as float, upper_bound_in.", _value);
                    return false;
                } catch (const std::out_of_range &ex) {
                    ::spdlog::info("could not parse in filter float_map_range the value {} as float, upper_bound_in.", _value);
                    return false;
                }
            }

            if("lower_bound_out" == key) {
                try {
                    this->lower_bound_out = std::stod(_value);
                    return true;
                } catch (const std::invalid_argument &ex) {
                    ::spdlog::info("could not parse in filter float_map_range the value {} as float, lower_bound_out.", _value);
                    return false;
                } catch (const std::out_of_range &ex) {
                    ::spdlog::info("could not parse in filter float_map_range the value {} as float, lower_bound_out.", _value);
                    return false;
                }
            }

            if("upper_bound_out" == key) {
                try {
                    this->upper_bound_out = std::stod(_value);
                    return true;
                } catch (const std::invalid_argument &ex) {
                    ::spdlog::info("could not parse in filter float_map_range the value {} as float, upper_bound_out.", _value);
                    return false;
                } catch (const std::out_of_range &ex) {
                    ::spdlog::info("could not parse in filter float_map_range the value {} as float, upper_bound_out.", _value);
                    return false;
                }
            }

            if("limit_range" == key) {
                this->limit = _value == "1";
                return true;
            }
            return false;
        }

        virtual void get_output_channels(channel_mapping& map, const std::string& name) override {
            if constexpr (std::is_same<T, uint8_t>::value) {
                map.eight_bit_channels[name + ":value"] = &output;
            } else if constexpr (std::is_same<T, uint16_t>::value) {
                map.sixteen_bit_channels[name + ":value"] = &output;
            } else if constexpr (std::is_same<T, double>::value) {
                map.float_channels[name + ":value"] = &output;
            }
        }

        virtual void update() override {
            double value_in = std::max(std::min(*input, this->limit ? upper_bound_in : *input), this->limit ? lower_bound_in : *input);
            double val = (value_in - lower_bound_in) / (upper_bound_in - lower_bound_in) * (upper_bound_out - lower_bound_out) + lower_bound_out;
            if constexpr (std::is_same<T, uint8_t>::value) {
                this->output = (uint8_t) std::round(std::max(std::min(val, 255.0), 0.0));
            } else if constexpr (std::is_same<T, uint16_t>::value) {
                this->output = (uint16_t) std::round(std::max(std::min(val, 65535.0), 0.0));
            } else if constexpr (std::is_same<T, double>::value) {
                this->output = val;
            }
        }

        virtual void scene_activated() override {}

    };

    using filter_float_map_range_8bit = float_map_range<uint8_t, filter_type::filter_float_map_range_8bit>;
    using filter_float_map_range_16bit = float_map_range<uint16_t, filter_type::filter_float_map_range_16bit>;
    using filter_float_map_range_float = float_map_range<double, filter_type::filter_float_map_range_float>;

    class filter_combine_bytes_to_16bit : public filter {
    private:
        uint16_t output = 0;
        uint8_t* lower_input = nullptr;
        uint8_t* upper_input = nullptr;
    public:
        filter_combine_bytes_to_16bit() : filter() {}
        virtual ~filter_combine_bytes_to_16bit() {}

        virtual void setup_filter(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const channel_mapping& input_channels, const std::string& own_id) override {
            MARK_UNUSED(initial_parameters);
            MARK_UNUSED(configuration);
            if(!input_channels.eight_bit_channels.contains("lower")) {
                throw filter_config_exception("Unable to link input of 8 bit merge to 16 bit filter: channel mapping does "
                                              "not contain channel 'lower' of type 'uint8_t'.",
                                              filter_type::filter_combine_bytes_to_16bit, own_id);
            }
            this->lower_input = input_channels.eight_bit_channels.at("lower");
            if(!input_channels.eight_bit_channels.contains("upper")) {
                throw filter_config_exception("Unable to link input of 8 bit merge to 16 bit filter: channel mapping does "
                                              "not contain channel 'upper' of type 'uint8_t'.",
                                              filter_type::filter_combine_bytes_to_16bit, own_id);
            }
            this->upper_input = input_channels.eight_bit_channels.at("upper");
        }

        virtual bool receive_update_from_gui(const std::string& key, const std::string& _value) override {
            MARK_UNUSED(key);
            MARK_UNUSED(_value);
            return false;
        }

        virtual void get_output_channels(channel_mapping& map, const std::string& name) override {
            map.sixteen_bit_channels[name + ":value"] = &output;
        }

        virtual void update() override {
            this->output = (uint16_t) (*this->upper_input << 8) + *this->lower_input;
        }

        virtual void scene_activated() override {}

    };

    class filter_map_8bit_to_16bit : public filter {
    private:
        uint16_t output = 0;
        uint8_t* input = nullptr;
    public:
        filter_map_8bit_to_16bit() : filter() {}
        virtual ~filter_map_8bit_to_16bit() {}

        virtual void setup_filter(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const channel_mapping& input_channels, const std::string& own_id) override {
            MARK_UNUSED(initial_parameters);
            MARK_UNUSED(configuration);
            if(!input_channels.eight_bit_channels.contains("value_in")) {
                throw filter_config_exception("Unable to link input of 8 bit map to 16 bit filter: channel mapping does "
                                              "not contain channel 'value_in' of type 'uint8_t'.",
                                              filter_type::filter_map_8bit_to_16bit, own_id);
            }
            this->input = input_channels.eight_bit_channels.at("value_in");
        }

        virtual bool receive_update_from_gui(const std::string& key, const std::string& _value) override {
            MARK_UNUSED(key);
            MARK_UNUSED(_value);
            return false;
        }

        virtual void get_output_channels(channel_mapping& map, const std::string& name) override {
            map.sixteen_bit_channels[name + ":value"] = &output;
        }

        virtual void update() override {
            this->output = (uint16_t) (*this->input << 8) + *this->input;
        }

        virtual void scene_activated() override {}

    };

COMPILER_RESTORE("-Weffc++")
}
