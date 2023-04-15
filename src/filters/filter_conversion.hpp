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
	    if(!input_channels.sixteen_bit_channels.contains("value")) {
		    throw filter_config_exception("Unable to link input of bool conversion filter: channel mapping does not contain channel 'value' of type 'uint16_t'.");
	    }
	    this->input = input_channels.sixteen_bit_channels.at("value");
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
            if(!input_channels.float_channels.contains("value")) {
                if constexpr (std::is_same<T, uint8_t>::value) {
                    throw filter_config_exception("Unable to link input of float to 16 bit conversion filter: channel mapping does not contain channel 'value' of type 'double'.");
                } else if constexpr (std::is_same<T, uint16_t>::value) {
                    throw filter_config_exception("Unable to link input of float to 8 bit conversion filter: channel mapping does not contain channel 'value' of type 'double'.");
                } else {
                    throw filter_config_exception("Unable to link input of number rounding filter: channel mapping does not contain channel 'value' of type 'double'.");
                }
            }
            this->input = input_channels.float_channels.at("value");
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
COMPILER_RESTORE("-Weffc++")
}
