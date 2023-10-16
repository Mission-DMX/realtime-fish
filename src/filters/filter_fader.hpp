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
#include "lib/logging.hpp"


namespace dmxfish::filters {

    COMPILER_SUPRESS("-Weffc++")
    template <dmxfish::control_desk::bank_mode MODE, typename STORAGE_T>
    class filter_fader_template : public filter {
    private:
        std::weak_ptr<dmxfish::control_desk::bank_column> input_col;
        STORAGE_T storage;
        bool first_retry = true;
        std::string set_id;
        std::string column_id;
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
            this->set_id = configuration.at("set_id");
            this->column_id = configuration.at("column_id");
            if(auto candidate = get_iomanager_instance()->access_desk_column(set_id, column_id); candidate) {
                if(const auto& column_mode = candidate->get_mode(); column_mode != MODE) {
                    std::stringstream ss;
		    ss << "The requested column (" << set_id << ":" << column_id << ") is not in the correct mode. Expected: ";
		    ss << (int) MODE << ", got: " << (int) column_mode << ".";
                    throw filter_config_exception(ss.str());
                }
                input_col = candidate;
                update();
            } else {
                std::stringstream ss;
                ss << "The requested column '" << column_id << "' in the '" << set_id << "' set does not seam to exist.";
                throw filter_config_exception(ss.str());
            }
            if constexpr (MODE != dmxfish::control_desk::bank_mode::DIRECT_INPUT_MODE) {
	        if(configuration.contains("ignore_main_brightness_control") && configuration.at("ignore_main_brightness_control") == "true") {
                    this->storage.global_main_enabled = false;
	        }
            }
        }

        bool reload_pointer() {
            if(auto candidate = get_iomanager_instance()->access_desk_column(set_id, column_id); candidate) {
                if(const auto& column_mode = candidate->get_mode(); column_mode != MODE) {
                    this->input_col = candidate;
                    ::spdlog::debug("Reloaded column pointer in fader filter.");
                    return true;
                }
            }
            this->first_retry = false;
            ::spdlog::warn("Failed to reload column pointer in fader filter.");
            return false;
        }

        virtual  bool receive_update_from_gui(const std::string& key, const std::string& _value) override {
            const auto old_set = this->set_id;
            const auto old_col = this->column_id;
            if(key == "set") {
                this->set_id = _value;
            } else if(key == "column") {
                this->column_id = _value;
            } else {
                return false;
            }
            if(!this->reload_pointer() && !this->input_col.expired()) {
                this->column_id = old_col;
                this->set_id = old_set;
                ::spdlog::warn("Reverting to old set and column id.");
            }
            first_retry = true;
            return true;
        }

        virtual void get_output_channels(channel_mapping& map, const std::string& name) override {
            using namespace dmxfish::control_desk;
            if constexpr (MODE == bank_mode::DIRECT_INPUT_MODE) {
                map.sixteen_bit_channels[name + ":primary"] = &(storage.primary_position);
                map.sixteen_bit_channels[name + ":secondary"] = &(storage.secondary_position);
            } else if constexpr (MODE == bank_mode::HSI_COLOR_MODE) {
                map.color_channels[name + ":color"] = &storage.color;
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
                    this->storage.color = col_ptr->get_color();
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
                if constexpr (MODE != bank_mode::DIRECT_INPUT_MODE) {
                    if(this->storage.global_main_enabled)
		        this->storage.color.iluminance *= (get_iomanager_instance()->get_global_illumination() / 65535);
                }
            } else {
                if (first_retry) {
                    reload_pointer();
                }
            }
        }

        virtual void scene_activated() override {
            first_retry = true;
        }

    };

    using filter_fader_column_raw = filter_fader_template<dmxfish::control_desk::bank_mode::DIRECT_INPUT_MODE, dmxfish::control_desk::raw_column_configuration>;

    struct filter_fader_column_hsi_storage { 
        dmxfish::dmx::pixel color;
	bool global_main_enabled = true;
    };
    using filter_fader_column_hsi = filter_fader_template<dmxfish::control_desk::bank_mode::HSI_COLOR_MODE, filter_fader_column_hsi_storage>;

    struct fader_column_hsia_storage_t : public filter_fader_column_hsi_storage {
        uint8_t amber;
    };

    using filter_fader_column_hsia = filter_fader_template<dmxfish::control_desk::bank_mode::HSI_WITH_AMBER_MODE, fader_column_hsia_storage_t>;

    struct fader_column_hsiu_storage_t : public filter_fader_column_hsi_storage {
        uint8_t uv;
    };

    using filter_fader_column_hsiu = filter_fader_template<dmxfish::control_desk::bank_mode::HSI_WITH_UV_MODE, fader_column_hsiu_storage_t>;

    struct fader_column_hsiau_storage_t : public fader_column_hsia_storage_t {
        uint8_t uv;
    };

    using filter_fader_column_hsiau = filter_fader_template<dmxfish::control_desk::bank_mode::HSI_WITH_AMBER_AND_UV_MODE, fader_column_hsiau_storage_t>;

    class filter_main_brightness_fader : public filter {
    private:
	uint16_t storage = 0;
    public:
        filter_main_brightness_fader() : filter() {}
        virtual ~filter_main_brightness_fader() {}

        virtual void setup_filter(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const channel_mapping& input_channels) override {
            MARK_UNUSED(configuration);
            MARK_UNUSED(input_channels);
            MARK_UNUSED(initial_parameters);
       }

        virtual  bool receive_update_from_gui(const std::string& key, const std::string& _value) override {
            MARK_UNUSED(key);
            MARK_UNUSED(_value);
            return false;
        }

        virtual void get_output_channels(channel_mapping& map, const std::string& name) override {
            map.sixteen_bit_channels[name + ":brightness"] = &storage;
        }

        virtual void update() override {
	    this->storage = get_iomanager_instance()->get_global_illumination();
	    //::spdlog::debug("{}", this->storage);
        }

        virtual void scene_activated() override {}


    };
    COMPILER_RESTORE("-Weffc++")
}
