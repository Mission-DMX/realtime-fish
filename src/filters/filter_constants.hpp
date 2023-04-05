#pragma once

/*
 * The filters defined in this file provide constant values on their output channel.
 * These filters should be able to be linked to the GUI as parameter inputs.
 */

#include <type_traits>

#include "dmx/pixel.hpp"
#include "filters/filter.hpp"
#include "lib/macros.hpp"


namespace dmxfish::filters {

    template <typename T>
    class filter_constant_template : public filter {
    private:
        T value;
    public:
        filter_constant_template() : filter(), value{} {
            static_assert(std::is_same<T, uint8_t>::value || std::is_same<T, uint16_t>::value || std::is_same<T, double>::value || std::is_same<T, dmxfish::dmx::pixel>::value, "unsupported data format.");
        }
        virtual ~filter_constant_template() {}

        virtual void setup_filter(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const channel_mapping& input_channels) override {
            MARK_UNUSED(input_channels);
            MARK_UNUSED(configuration);
            if(!initial_parameters.contains("value") || !this->receive_update_from_gui("value", initial_parameters.at("value"))) {
                throw filter_config_exception("Unable to set value of constant.");
            }
        }

        virtual  bool receive_update_from_gui(const std::string& key, const std::string& _value) override {
            if(key != "value") {
                return false;
            }
            try {
                if constexpr (std::is_same<T, uint8_t>::value || std::is_same<T, uint16_t>::value) {
                    this->value = (T) std::stoi(_value);
                } else if constexpr (std::is_same<T, double>::value) {
                    this->value = std::stod(_value);
                } else {
                    const auto first_position = _value.find(",");
                    this->value.hue = std::stod(_value.substr(0, first_position));
                    const auto second_position = _value.find(",", first_position + 1);
                    this->value.saturation = std::stod(_value.substr(first_position + 1, second_position - first_position - 1));
                    this->value.value = std::stod(_value.substr(second_position + 1));
                }
            } catch (const std::invalid_argument& ex) {
                MARK_UNUSED(ex);
                return false;
            } catch (const std::out_of_range& ex) {
                MARK_UNUSED(ex);
                return false;
            }
            return true;
        }

        virtual void get_output_channels(channel_mapping& map, const std::string& name) override {
            if constexpr (std::is_same<T, uint8_t>::value) {
                map.eight_bit_channels[name + ":value"] = &value;
            } else if constexpr (std::is_same<T, uint16_t>::value) {
                map.sixteen_bit_channels[name + ":value"] = &value;
            } else if constexpr (std::is_same<T, double>::value) {
                map.float_channels[name + ":value"] = &value;
            } else {
                map.color_channels[name + ":value"] = &value;
            }
        }

        virtual void update() override {}

        virtual void scene_activated() override {}

    };

    using constant_8bit = filter_constant_template<uint8_t>;
    using constant_16bit = filter_constant_template<uint16_t>;
    using constant_float = filter_constant_template<double>;
    using constant_color = filter_constant_template<dmxfish::dmx::pixel>;
}
