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

    template <typename T, filter_type own_type>
    class filter_constant_template : public filter {
    private:
        T value;
    public:
        filter_constant_template() : filter(), value{} {
            static_assert(std::is_same<T, uint8_t>::value || std::is_same<T, uint16_t>::value || std::is_same<T, double>::value || std::is_same<T, dmxfish::dmx::pixel>::value, "unsupported data format.");
        }
        virtual ~filter_constant_template() {}

        virtual void setup_filter(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const channel_mapping& input_channels, const std::string& own_id) override {
            MARK_UNUSED(input_channels);
            MARK_UNUSED(configuration);
            if(!initial_parameters.contains("value")){
                throw filter_config_exception("Unable to set value of constant filter: initial configuration does not contain value parameter", own_type, own_id);
            } else if (!this->receive_update_from_gui("value", initial_parameters.at("value"))) {
                throw filter_config_exception("Unable to set value of constant filter: unable to parse parameter.", own_type, own_id);
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
                    this->value = dmxfish::dmx::stopixel(_value);
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

    using constant_8bit = filter_constant_template<uint8_t, filter_type::constants_8bit>;
    using constant_16bit = filter_constant_template<uint16_t, filter_type::constants_16bit>;
    using constant_float = filter_constant_template<double, filter_type::constants_float>;
    using constant_color = filter_constant_template<dmxfish::dmx::pixel, filter_type::constants_pixel>;
}
