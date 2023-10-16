#pragma once

/*
 * The filters defined in this file provide adapters to other formats.
 */

#include <cmath>

#include "filters/filter.hpp"
#include "lib/macros.hpp"


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

        virtual void setup_filter(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const channel_mapping& input_channels) override {
            MARK_UNUSED(initial_parameters);
            MARK_UNUSED(configuration);
            if(!input_channels.sixteen_bit_channels.contains("value")) {
                throw filter_config_exception("Unable to link input of 16 bit splitting filter: channel mapping does not contain channel 'value' of type 'uint16_t'.");
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

        virtual void setup_filter(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const channel_mapping& input_channels) override {
            MARK_UNUSED(initial_parameters);
            MARK_UNUSED(configuration);
            if(!input_channels.sixteen_bit_channels.contains("value_in")) {
                throw filter_config_exception("Unable to link input of bool conversion filter: channel mapping does not contain channel 'value_in' of type 'uint16_t'.");
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

        virtual void setup_filter(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const channel_mapping& input_channels) override {
            MARK_UNUSED(initial_parameters);
            MARK_UNUSED(configuration);
            if(!input_channels.float_channels.contains("factor1") ||
                    !input_channels.float_channels.contains("factor2") ||
                    !input_channels.float_channels.contains("summand")) {
                throw filter_config_exception("Unable to link input of multiply add filter: channel mapping does not contain required channels.");
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

    template <typename T>
    class filter_float_to_X_template : public filter {
    private:
        double* input = nullptr;
        T output = 0;
    public:
        filter_float_to_X_template() : filter() {
            static_assert(std::is_same<T, uint8_t>::value || std::is_same<T, uint16_t>::value || std::is_same<T, double>::value, "unsupported data format.");
        }
        virtual ~filter_float_to_X_template() {}

        virtual void setup_filter(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const channel_mapping& input_channels) override {
            MARK_UNUSED(initial_parameters);
            MARK_UNUSED(configuration);
            if(!input_channels.float_channels.contains("value_in")) {
                if constexpr (std::is_same<T, uint8_t>::value) {
                    throw filter_config_exception("Unable to link input of float to 16 bit conversion filter: channel mapping does not contain channel 'value_in' of type 'double'.");
                } else if constexpr (std::is_same<T, uint16_t>::value) {
                    throw filter_config_exception("Unable to link input of float to 8 bit conversion filter: channel mapping does not contain channel 'value_in' of type 'double'.");
                } else {
                    throw filter_config_exception("Unable to link input of number rounding filter: channel mapping does not contain channel 'value_in' of type 'double'.");
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

    using filter_float_to_16bit = filter_float_to_X_template<uint16_t>;
    using filter_float_to_8bit = filter_float_to_X_template<uint8_t>;
    using filter_round_number = filter_float_to_X_template<double>;

    class filter_pixel_to_rgb_channels : public filter {
    private:
        dmxfish::dmx::pixel* input = nullptr;
        uint8_t r = 0, g = 0, b = 0;
    public:
        filter_pixel_to_rgb_channels() : filter() {}
        virtual ~filter_pixel_to_rgb_channels() {}

        virtual void setup_filter(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const channel_mapping& input_channels) override {
            MARK_UNUSED(initial_parameters);
            MARK_UNUSED(configuration);
            if(!input_channels.color_channels.contains("value")) {
                throw filter_config_exception("Unable to link input of color to rgb filter: channel mapping does not contain channel 'value' of type 'color'.");
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
            double H = input->hue, S = input->saturation, I = input->iluminance;
            H = std::fmod(H, 360);
            H = 3.14159*H / (double) 180;
            S = S>0 ? (S<1 ? S : 1) : 0;
            I = I>0 ? (I<1 ? I : 1) : 0;

            if(H < 2.09439) {
                r = (uint8_t) std::round(255*I/3*(1+S*std::cos(H)/std::cos(1.047196667-H)));
                g = (uint8_t) std::round(255*I/3*(1+S*(1-std::cos(H)/std::cos(1.047196667-H))));
                b = (uint8_t) std::round(255*I/3*(1-S));
            } else if(H < 4.188787) {
                H = H - 2.09439;
                g = (uint8_t) std::round(255*I/3*(1+S*std::cos(H)/std::cos(1.047196667-H)));
                b = (uint8_t) std::round(255*I/3*(1+S*(1-std::cos(H)/std::cos(1.047196667-H))));
                r = (uint8_t) std::round(255*I/3*(1-S));
            } else {
                H = H - 4.188787;
                b = (uint8_t) std::round(255*I/3*(1+S*std::cos(H)/std::cos(1.047196667-H)));
                r = (uint8_t) std::round(255*I/3*(1+S*(1-std::cos(H)/std::cos(1.047196667-H))));
                g = (uint8_t) std::round(255*I/3*(1-S));
            }
//            input->pixel_to_rgb(r, g, b);
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

        virtual void setup_filter(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const channel_mapping& input_channels) override {
            MARK_UNUSED(initial_parameters);
            MARK_UNUSED(configuration);
            if(!input_channels.color_channels.contains("value")) {
                throw filter_config_exception("Unable to link input of color to rgbw filter: channel mapping does not contain channel 'value' of type 'color'.");
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
            double H = input->hue, S = input->saturation, I = input->iluminance;
            double cos_h, cos_1047_h;

            H = std::fmod(H,360);
            H = 3.14159*H/(double)180;
            S = S>0 ? (S<1 ? S : 1) : 0;
            I = I>0 ? (I<1 ? I : 1) : 0;

            if(H < 2.09439) {
                cos_h = std::cos(H);
                cos_1047_h = std::cos(1.047196667-H);
                r = (uint8_t) std::round(S*255*I / 3*(1+cos_h/cos_1047_h));
                g = (uint8_t) std::round(S*255*I / 3*(1+(1-cos_h/cos_1047_h)));
                b = 0;
                w = (uint8_t) std::round(255*(1-S)*I);
            } else if(H < 4.188787) {
                H = H - 2.09439;
                cos_h = std::cos(H);
                cos_1047_h = std::cos(1.047196667-H);
                g = (uint8_t) std::round(S*255*I / 3*(1+cos_h/cos_1047_h));
                b = (uint8_t) std::round(S*255*I / 3*(1+(1-cos_h/cos_1047_h)));
                r = 0;
                w = (uint8_t) std::round(255*(1-S)*I);
            } else {
                H = H - 4.188787;
                cos_h = std::cos(H);
                cos_1047_h = std::cos(1.047196667-H);
                b = (uint8_t) std::round(S*255*I / 3*(1+cos_h/cos_1047_h));
                r = (uint8_t) std::round(S*255*I / 3*(1+(1-cos_h/cos_1047_h)));
                g = 0;
                w = (uint8_t) std::round(255*(1-S)*I);
            }
//            input->pixel_to_rgb(r, g, b, w);
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

        virtual void setup_filter(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const channel_mapping& input_channels) override {
            MARK_UNUSED(initial_parameters);
            MARK_UNUSED(configuration);
            if(!input_channels.float_channels.contains("h") || !input_channels.float_channels.contains("s") || !input_channels.float_channels.contains("i")) {
                throw filter_config_exception("Unable to link input of float to color filter: channel mapping does not contain input channels.");
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

        virtual void setup_filter(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const channel_mapping& input_channels) override {
            MARK_UNUSED(initial_parameters);
            MARK_UNUSED(configuration);
            if(!input_channels.color_channels.contains("input")) {
                throw filter_config_exception("Unable to link input of color to float filter: channel mapping does not contain the input channel.");
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

    template <typename T>
    class filter_to_float_template : public filter {
    private:
        T* input = nullptr;
        double output = 0.0;
    public:
        filter_to_float_template() : filter() {}
        virtual ~filter_to_float_template() {}

        virtual void setup_filter(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const channel_mapping& input_channels) override {
            MARK_UNUSED(initial_parameters);
            MARK_UNUSED(configuration);

            if constexpr (std::is_same<T, uint8_t>::value) {
                if(!input_channels.eight_bit_channels.contains("value_in")) {
                    throw filter_config_exception("Unable to link input of float conversion filter: channel mapping does not contain channel 'value_in' of type 'uint8_t'.");
                }
                this->input = input_channels.eight_bit_channels.at("value_in");
            } else if constexpr (std::is_same<T, uint16_t>::value) {
                if(!input_channels.sixteen_bit_channels.contains("value_in")) {
                    throw filter_config_exception("Unable to link input of float conversion filter: channel mapping does not contain channel 'value_in' of type 'uint16_t'.");
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

    using filter_8bit_to_float = filter_to_float_template<uint8_t>;
    using filter_16bit_to_float = filter_to_float_template<uint16_t>;


COMPILER_RESTORE("-Weffc++")
}
