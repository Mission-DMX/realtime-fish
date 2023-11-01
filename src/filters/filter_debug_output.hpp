#pragma once

/*
 * The filters defined in this file log their received values once every iteration.
 * Use them with care as their execution is quite expensive.
 */

#include <type_traits>

#include "dmx/pixel.hpp"
#include "filters/filter.hpp"
#include "lib/logging.hpp"
#include "lib/macros.hpp"


namespace dmxfish::filters {
COMPILER_SUPRESS("-Weffc++")
    template <typename T, filter_type own_type>
    class filter_debug_output_template : public filter {
    private:
        T* input = nullptr;
        std::string filter_name;
    public:
        filter_debug_output_template() : filter(), filter_name() {
            static_assert(std::is_same<T, uint8_t>::value || std::is_same<T, uint16_t>::value || std::is_same<T, double>::value || std::is_same<T, dmxfish::dmx::pixel>::value, "unsupported data format.");
        }
        virtual ~filter_debug_output_template() {}

        virtual void setup_filter(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const channel_mapping& input_channels, const std::string& own_id) override {
            MARK_UNUSED(initial_parameters);
            MARK_UNUSED(configuration);
            // TODO use configuration to push received values to GUI
            if constexpr (std::is_same<T, uint8_t>::value) {
                if(!input_channels.eight_bit_channels.contains("value")) {
                    throw filter_config_exception("Unable to link input of debug output filter: channel mapping does "
                                                  "not contain channel 'value' of type 'uint8_t'.", own_type, own_id);
                }
                this->input = input_channels.eight_bit_channels.at("value");
            } else if constexpr (std::is_same<T, uint16_t>::value) {
                if(!input_channels.sixteen_bit_channels.contains("value")) {
                    throw filter_config_exception("Unable to link input of debug output filter: channel mapping does "
                                                  "not contain channel 'value' of type 'uint16_t'.", own_type, own_id);
                }
                this->input = input_channels.sixteen_bit_channels.at("value");
            } else if constexpr (std::is_same<T, double>::value) {
                if(!input_channels.float_channels.contains("value")) {
                    throw filter_config_exception("Unable to link input of debug output filter: channel mapping does "
                                                  "not contain channel 'value' of type 'double'.", own_type, own_id);
                }
                this->input = input_channels.float_channels.at("value");
            } else {
                if(!input_channels.color_channels.contains("value")) {
                    throw filter_config_exception("Unable to link input of debug output filter: channel mapping does "
                                                  "not contain channel 'value' of type 'hsv_pixel'.", own_type, own_id);
                }
                this->input = input_channels.color_channels.at("value");
            }

        }

        virtual  bool receive_update_from_gui(const std::string& key, const std::string& _value) override {
            MARK_UNUSED(key);
            MARK_UNUSED(_value);
            return false;
        }

        virtual void get_output_channels(channel_mapping& map, const std::string& name) override {
            MARK_UNUSED(map);
            this->filter_name = name;
        }

        virtual void update() override {
            if(this->input != nullptr) {
                if constexpr (std::is_same<T, dmxfish::dmx::pixel>::value) {
                    ::spdlog::debug("Output from {} = {}.", this->filter_name, input->str());
                } else {
                    ::spdlog::debug("Output from {} = {}.", this->filter_name, *input);
                }
            } else {
                ::spdlog::error("Input channel of filter {} is null.", this->filter_name);
            }
        }

        virtual void scene_activated() override {}

    };

    using debug_8bit = filter_debug_output_template<uint8_t, filter_type::debug_8bit>;
    using debug_16bit = filter_debug_output_template<uint16_t, filter_type::debug_16bit>;
    using debug_float = filter_debug_output_template<double, filter_type::debug_float>;
    using debug_pixel = filter_debug_output_template<dmxfish::dmx::pixel, filter_type::debug_pixel>;
COMPILER_RESTORE("-Weffc++")
}
