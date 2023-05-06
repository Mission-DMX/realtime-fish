#pragma once

/*
 * The filters defined in this file provide access the fader inputs.
 */

#include <memory>

#include "dmx/pixel.hpp"
#include "filters/filter.hpp"
#include "lib/macros.hpp"

#include "control_desk/bank_column.hpp"


namespace dmxfish::filters {

    template <bank_mode MODE>
    class filter_fader_template : public filter {
    private:
        std::weak_ptr<bank_column> input_col = nullptr;
        dmxfish::dmx::pixel color;
        uint8_t uv;
        uint8_t amber;
        uint16_t fader_position;
        uint16_t rotary_position;
    public:
        filter_fader_template() : filter(), value{} {}
        virtual ~filter_fader_template() {}

        virtual void setup_filter(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const channel_mapping& input_channels) override {
            MARK_UNUSED(input_channels);
            MARK_UNUSED(initial_parameters);
            if(!configuration.contains("column_id")){
                throw filter_config_exception("Unable to set value of fader input filter: configuration does not contain column_id parameter");
            }
            static_assert(false, "Retrival of column not yet implemented.");
        }

        virtual  bool receive_update_from_gui(const std::string& key, const std::string& _value) override {
            return false;
        }

        virtual void get_output_channels(channel_mapping& map, const std::string& name) override {
            if constexpr (MODE == bank_mode::DIRECT_INPUT_MODE) {
                map.sixteen_bit_channels[name + ":fader"] = &fader_position;
                map.sixteen_bit_channels[name + ":encoder"] = &rotary_position;
            } else if constexpr (MODE == bank_mode::HSI_COLOR_MODE) {
                map.color_channels[name + ":color"] = &color;
            } else if constexpr (MODE == bank_mode::HSI_WITH_AMBER_MODE) {
                map.color_channels[name + ":color"] = &color;
                map.eight_bit_channels[name + ":amber"] = &amber;
            } else if constexpr (MODE == bank_mode::HSI_WITH_UV_MODE) {
                map.color_channels[name + ":color"] = &color;
                map.eight_bit_channels[name + ":uv"] = &uv;
            } else {
                map.color_channels[name + ":color"] = &color;
                map.eight_bit_channels[name + ":amber"] = &amber;
                map.eight_bit_channels[name + ":uv"] = &uv;
            }
        }

        virtual void update() override {
            if(auto col_ptr = input_col.lock()) {
                if constexpr (MODE == bank_mode::DIRECT_INPUT_MODE) {
                    const auto& raw_data = col_ptr->get_raw_configuration();
                    this->fader_position = raw_data.fader_position;
                    this->rotary_position = raw_data.rotary_position;
                } else if constexpr (MODE == bank_mode::HSI_COLOR_MODE) {
                    this->color = col_ptr->get_color();
                } else if constexpr (MODE == bank_mode::HSI_WITH_AMBER_MODE) {
                    this->color = col_ptr->get_color();
                    this->amber = col_ptr->get_amber_value();
                } else if constexpr (MODE == bank_mode::HSI_WITH_UV_MODE) {
                    this->color = col_ptr->get_color();
                    this->uv = col_ptr->get_uv_value();
                } else {
                    this->color = col_ptr->get_color();
                    this->amber = col_ptr->get_amber_value();
                    this->uv = col_ptr->get_uv_value();
                }
            }
        }

        virtual void scene_activated() override {}

    };

    using fader_column_raw = filter_fader_template<DIRECT_INPUT_MODE>;
    using fader_column_hsi = filter_fader_template<HSI_COLOR_MODE>;
    using fader_column_hsia = filter_fader_template<HSI_WITH_AMBER_MODE>;
    using fader_column_hsiu = filter_fader_template<HSI_WITH_UV_MODE>;
    using fader_column_hsiau = filter_fader_template<HSI_WITH_AMBER_AND_UV_MODE>;
}
