#pragma once

/*
 * The filters defined in this file provide constant values on their output channel.
 * These filters should be able to be linked to the GUI as parameter inputs.
 */

#include <map>
#include <memory>
#include <string>
#include <type_traits>

#include "dmx/pixel.hpp"
#include "filters/filter.hpp"
#include "lib/macros.hpp"
#include "../main.hpp"

#include "proto_src/MessageTypes.pb.h"
#include "proto_src/FilterMode.pb.h"

namespace dmxfish::filters {

    template <typename T, filter_type own_type>
    class filter_constant_template : public filter {
    protected:
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

        virtual bool receive_update_from_gui(const std::string& key, const std::string& _value) override {
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

    template <typename T, filter_type own_type>
    class responding_constant_filter_template : public filter_constant_template<T, own_type> {
    private:
        std::string own_id;
    public:
        responding_constant_filter_template() : filter_constant_template<T, own_type>() {}

        virtual void setup_filter(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const channel_mapping& input_channels, const std::string& _own_id) override {
            filter_constant_template<T, own_type>::setup_filter(configuration, initial_parameters, input_channels, _own_id);
            this->own_id = _own_id;
        }

        virtual bool receive_update_from_gui(const std::string& key, const std::string& _value) override {
            auto ret_val = filter_constant_template<T, own_type>::receive_update_from_gui(key, _value);
            this->push_value_update();
            return ret_val;
        }

        virtual void scene_activated() override {
            filter_constant_template<T, own_type>::scene_activated();
            this->push_value_update();
        }
    private:
        void push_value_update() const {
            if (auto iomanager = get_iomanager_instance(); iomanager != nullptr) {
                if (auto s = iomanager->get_active_show(); s != nullptr) {
                    auto update_message = missiondmx::fish::ipcmessages::update_parameter();
                    update_message.set_filter_id(this->own_id);
                    update_message.set_parameter_key("value");
                    update_message.set_scene_id(s->get_current_scene_id());
                    update_message.set_parameter_value(std::to_string(this->value));
                    iomanager->push_msg_to_all_gui(update_message, ::missiondmx::fish::ipcmessages::MSGT_UPDATE_PARAMETER);
                }
            }
        }
    };

    using responding_constant_8bit = responding_constant_filter_template<uint8_t, filter_type::constants_8bit>;
    using responding_constant_16bit = responding_constant_filter_template<uint16_t, filter_type::constants_16bit>;
    using responding_constant_float = responding_constant_filter_template<double, filter_type::constants_float>;
    using responding_constant_color = responding_constant_filter_template<dmxfish::dmx::pixel, filter_type::constants_pixel>;

}
