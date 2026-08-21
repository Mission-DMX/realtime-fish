#pragma once

/*
 * This filter selects one of the inputs and forwards it to the outputs.
 */

#include <vector>

#include "filters/filter.hpp"
#include "lib/macros.hpp"

#include "lib/logging.hpp"

namespace dmxfish::filters {

    COMPILER_SUPRESS("-Weffc++")

    template <typename T, filter_type own_type>
    class filter_switch: public filter {
    private:
        T output;
        uint8_t* select_input = nullptr;
        std::vector<T*> inputs;
    public:
        filter_switch() : filter() {}
        virtual ~filter_switch() {}

	virtual void pre_setup(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const std::string& own_id) {
	    if (!configuration.contains("nr_inputs")){
                throw filter_config_exception("Unable to setup switch filter: configuration does not contain a value "
                                              "for 'nr_inputs'", own_type, own_id);
            }
            int nr_inputs = 0;
            try {
                nr_inputs = std::stoi(configuration.at("nr_inputs"));
            } catch (const std::invalid_argument& ex) {
                MARK_UNUSED(ex);
                throw filter_config_exception("Unable to setup shift filter: could not parse the 'nr_inputs' as int",
                                              own_type, own_id);
            }
            if (nr_inputs < 1) {
                throw filter_config_exception("Unable to setup shift filter: the number of inputs is less then 1",
                                              own_type, own_id);
            }
            this->inputs.resize(nr_inputs);
	}

        virtual void setup_filter(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const channel_mapping& input_channels, const std::string& own_id) override {
            MARK_UNUSED(initial_parameters);

            for(auto i = 0; i < this->inputs.size(); i++) {
                    if constexpr (std::is_same<T, uint8_t>::value) {
                        if(!input_channels.eight_bit_channels.contains(std::to_string(i))) {
                            throw filter_config_exception("Unable to link input of shift filter: channel mapping does not "
                                                          "contain channel input of type 'uint8_t' for " + std::to_string(i) + ".", own_type, own_id);
                        }
                        this->inputs[i] = input_channels.eight_bit_channels.at(std::to_string(i));
                    } else if constexpr (std::is_same<T, uint16_t>::value) {
                        if(!input_channels.sixteen_bit_channels.contains(std::to_string(i))) {
                            throw filter_config_exception("Unable to link input of shift filter: channel mapping does not "
                                                          "contain channel input of type 'uint16_t' for " + std::to_string(i) + ".", own_type, own_id);
                        }
                        this->inputs[i] = input_channels.sixteen_bit_channels.at(std::to_string(i));
                    } else if constexpr (std::is_same<T, double>::value) {
                        if(!input_channels.float_channels.contains(std::to_string(i))) {
                            throw filter_config_exception("Unable to link input of shift filter: channel mapping does not "
                                                          "contain channel input of type 'double' for " + std::to_string(i) + ".", own_type, own_id);
                        }
                        this->inputs[i] = input_channels.float_channels.at(std::to_string(i));
                    } else {
                        if(!input_channels.color_channels.contains(std::to_string(i))) {
                            throw filter_config_exception("Unable to link input of shift filter: channel mapping does not "
                                                          "contain channel input of type 'hsv_pixel' for " + std::to_string(i) + ".", own_type, own_id);
                        }
                        this->inputs[i] = input_channels.color_channels.at(std::to_string(i));
                    }
            }
            if(!input_channels.eight_bit_channels.contains("select")) {
                throw filter_config_exception("Unable to link input of shift filter: channel mapping does not "
                                              "contain channel `select` of type 'uint8_t'.", own_type, own_id);
            }
            this->select_input = input_channels.eight_bit_channels.at("select");
        }

        virtual bool receive_update_from_gui(const std::string& key, const std::string& _value) override {
            MARK_UNUSED(key);
            MARK_UNUSED(_value);
            return false;
        }

        virtual void get_output_channels(channel_mapping& map, const std::string& name) override {
            if constexpr (std::is_same<T, uint8_t>::value) {
                map.eight_bit_channels["out"] = &(this->output);
            } else if constexpr (std::is_same<T, uint16_t>::value) {
                map.sixteen_bit_channels["out"] = &(this->output);
            } else if constexpr (std::is_same<T, double>::value) {
                map.float_channels["out"] = &(this->output);
            } else {
                map.color_channels["out"] = &(this->output);
            }
	}

        virtual void update() override {
            const auto selected_input = *select_input;
            if (selected_input >= this->inputs.size()) {
                return;
            }
            this->output = *(this->inputs[selected_input]);
        }

        virtual void scene_activated() override {}

    };

    using filter_switch_8bit = filter_switch<uint8_t, filter_type::filter_shift_8bit>;
    using filter_switch_16bit = filter_switch<uint16_t, filter_type::filter_shift_16bit>;
    using filter_switch_float = filter_switch<double, filter_type::filter_shift_16bit>;
    using filter_switch_color = filter_switch<dmxfish::dmx::pixel, filter_type::filter_shift_16bit>;

    COMPILER_RESTORE("-Weffc++");

}
