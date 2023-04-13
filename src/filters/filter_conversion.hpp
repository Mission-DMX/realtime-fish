#pragma once

/*
 * The filters defined in this file provide adapters to other formats.
 */

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
COMPILER_RESTORE("-Weffc++")
}
