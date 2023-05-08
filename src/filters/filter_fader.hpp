#pragma once

/*
 * The filters defined in this file provide access the fader inputs.
 */

#include <memory>
#include <sstream>

#include "dmx/pixel.hpp"
#include "filters/filter.hpp"
#include "lib/macros.hpp"
#include "main.hpp"

#include "control_desk/bank_column.hpp"


namespace dmxfish::filters {

    COMPILER_SUPRESS("-Weffc++")
    template <dmxfish::control_desk::bank_mode MODE, typename STORAGE_T>
    class filter_fader_template : public filter {
    private:
        std::weak_ptr<dmxfish::control_desk::bank_column> input_col;
        STORAGE_T storage;
    public:
        filter_fader_template() : filter(), input_col{}, storage{} {}
        virtual ~filter_fader_template() {}

        virtual void setup_filter(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const channel_mapping& input_channels) override {
            MARK_UNUSED(input_channels);
            MARK_UNUSED(initial_parameters);
            if(!configuration.contains("column_id")){
                throw filter_config_exception("Unable to set value of fader input filter: configuration does not contain column_id parameter");
            }
            if(!configuration.contains("set_id")){
                throw filter_config_exception("Unable to set value of fader input filter: configuration does not contain set_id parameter");
            }
            const auto& set_id = configuration.at("set_id");
            const auto& column_id = configuration.at("column_id");
            if(auto candidate = get_iomanager_instance()->access_desk_column(set_id, column_id); candidate) {
                input_col = candidate;
                update();
            } else {
                std::stringstream ss;
                ss << "The requested column '" << column_id << "' in the '" << set_id << "' set does not seam to exist.";
                throw filter_config_exception(ss.str());
            }
        }

        virtual  bool receive_update_from_gui(const std::string& key, const std::string& _value) override {
            return false;
        }

        virtual void get_output_channels(channel_mapping& map, const std::string& name) override {
            using namespace dmxfish::control_desk;
            if constexpr (MODE == bank_mode::DIRECT_INPUT_MODE) {
                map.sixteen_bit_channels[name + ":fader"] = &(storage.fader_position);
                map.sixteen_bit_channels[name + ":encoder"] = &(storage.rotary_position);
            } else if constexpr (MODE == bank_mode::HSI_COLOR_MODE) {
                map.color_channels[name + ":color"] = &storage;
            } else if constexpr (MODE == bank_mode::HSI_WITH_AMBER_MODE) {
                map.color_channels[name + ":color"] = &(storage.color);
                map.eight_bit_channels[name + ":amber"] = &(storage.amber);
            } else if constexpr (MODE == bank_mode::HSI_WITH_UV_MODE) {
                map.color_channels[name + ":color"] = &(storage.color);
                map.eight_bit_channels[name + ":uv"] = &(storage.uv);
            } else {
                map.color_channels[name + ":color"] = &(storage.color);
                map.eight_bit_channels[name + ":amber"] = &(storage.amber);
                map.eight_bit_channels[name + ":uv"] = &(storage.uv);
            }
        }

        virtual void update() override {
            if(auto col_ptr = input_col.lock()) {
                using namespace dmxfish::control_desk;
                if constexpr (MODE == bank_mode::DIRECT_INPUT_MODE) {
                    this->storage = col_ptr->get_raw_configuration();
                } else if constexpr (MODE == bank_mode::HSI_COLOR_MODE) {
                    this->storage = col_ptr->get_color();
                } else if constexpr (MODE == bank_mode::HSI_WITH_AMBER_MODE) {
                    this->storage.color = col_ptr->get_color();
                    this->storage.amber = col_ptr->get_amber_value();
                } else if constexpr (MODE == bank_mode::HSI_WITH_UV_MODE) {
                    this->storage.color = col_ptr->get_color();
                    this->storage.uv = col_ptr->get_uv_value();
                } else {
                    this->storage.color = col_ptr->get_color();
                    this->storage.amber = col_ptr->get_amber_value();
                    this->storage.uv = col_ptr->get_uv_value();
                }
            }
        }

        virtual void scene_activated() override {}

    };

    using filter_fader_column_raw = filter_fader_template<dmxfish::control_desk::bank_mode::DIRECT_INPUT_MODE, dmxfish::control_desk::raw_column_configuration>;
    using filter_fader_column_hsi = filter_fader_template<dmxfish::control_desk::bank_mode::HSI_COLOR_MODE, dmxfish::dmx::pixel>;

    struct fader_column_hsia_storage_t {
        dmxfish::dmx::pixel color;
        uint8_t amber;
    };

    using filter_fader_column_hsia = filter_fader_template<dmxfish::control_desk::bank_mode::HSI_WITH_AMBER_MODE, fader_column_hsia_storage_t>;

    struct fader_column_hsiu_storage_t {
        dmxfish::dmx::pixel color;
        uint8_t uv;
    };

    using filter_fader_column_hsiu = filter_fader_template<dmxfish::control_desk::bank_mode::HSI_WITH_UV_MODE, fader_column_hsiu_storage_t>;

    struct fader_column_hsiau_storage_t : public fader_column_hsia_storage_t {
        uint8_t uv;
    };

    using filter_fader_column_hsiau = filter_fader_template<dmxfish::control_desk::bank_mode::HSI_WITH_AMBER_AND_UV_MODE, fader_column_hsiau_storage_t>;
    COMPILER_RESTORE("-Weffc++")
}
