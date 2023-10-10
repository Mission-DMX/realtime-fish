#pragma once

/*
 * The filters defined in this file log their received values once every iteration.
 * Use them with care as their execution is quite expensive.
 */

#include <type_traits>

#include "dmx/pixel.hpp"
#include "filters/filter.hpp"
#include "filters/util.hpp"
#include "lib/logging.hpp"
#include "lib/macros.hpp"


namespace dmxfish::filters {
COMPILER_SUPRESS("-Weffc++")
    template <typename T>
    class filter_debug_output_template : public filter {
    private:
        T* input = nullptr;
        std::string filter_name;
    public:
        filter_debug_output_template() : filter(), filter_name() {
            static_assert(std::is_same<T, uint8_t>::value || std::is_same<T, uint16_t>::value || std::is_same<T, double>::value || std::is_same<T, dmxfish::dmx::pixel>::value, "unsupported data format.");
        }
        virtual ~filter_debug_output_template() {}

        virtual void setup_filter(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const channel_mapping& input_channels) override {
            MARK_UNUSED(initial_parameters);
            MARK_UNUSED(configuration);
            // TODO use configuration to push received values to GUI
            if constexpr (std::is_same<T, uint8_t>::value) {
                this->input = input_channels.eight_bit_channels.contains("value") ? input_channels.eight_bit_channels.at("value") : &util::low_8bit;
            } else if constexpr (std::is_same<T, uint16_t>::value) {
                this->input = input_channels.sixteen_bit_channels.contains("value") ? input_channels.sixteen_bit_channels.at("value") : &util::low_16bit;
            } else if constexpr (std::is_same<T, double>::value) {
                this->input = input_channels.float_channels.contains("value") ? input_channels.float_channels.at("value") : &util::float_zero;
            } else {
                this->input = input_channels.color_channels.contains("value") ? input_channels.color_channels.at("value") : &util::color_white;
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

    using debug_8bit = filter_debug_output_template<uint8_t>;
    using debug_16bit = filter_debug_output_template<uint16_t>;
    using debug_float = filter_debug_output_template<double>;
    using debug_pixel = filter_debug_output_template<dmxfish::dmx::pixel>;
COMPILER_RESTORE("-Weffc++")
}
