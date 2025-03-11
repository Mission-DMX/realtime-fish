#pragma once

/*
 * The filters calculate basic math functions
 */

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>
#include <string>
#include <type_traits>

#include "filters/filter.hpp"
#include "lib/macros.hpp"


namespace dmxfish::filters {

    COMPILER_SUPRESS("-Weffc++")
    template <double (*F)(double), filter_type own_type>
    class filter_math_single: public filter {
    private:
        double* input = nullptr;
        double output = 0;
    public:
        filter_math_single() : filter() {}
        virtual ~filter_math_single() {}

        virtual void setup_filter(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const channel_mapping& input_channels, const std::string& own_id) override {
            MARK_UNUSED(initial_parameters);
            MARK_UNUSED(configuration);
            if(!input_channels.float_channels.contains("value_in")) {
                throw filter_config_exception("Unable to link input of math filter: channel mapping does not contain "
                                              "channel 'value_in' of type 'double'.", own_type, own_id);
            }
            this->input = input_channels.float_channels.at("value_in");
        }

        virtual bool receive_update_from_gui(const std::string& key, const std::string& _value) override {
            MARK_UNUSED(key);
            MARK_UNUSED(_value);
            return false;
        }

        virtual void get_output_channels(channel_mapping& map, const std::string& name) override {
            map.float_channels[name + ":value"] = &output;
        }

        virtual void update() override {
            this->output = (*input);
        }

        virtual void scene_activated() override {}

    };

    template <typename T, T (*F)(T, T), filter_type own_type>
    class filter_math_dual: public filter {
    private:
        T* param1 = nullptr;
        T* param2 = nullptr;
        T output = 0;
    public:
        filter_math_dual() : filter() {}
        virtual ~filter_math_dual() {}

        virtual void setup_filter(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const channel_mapping& input_channels, const std::string& own_id) override {
            MARK_UNUSED(initial_parameters);
            MARK_UNUSED(configuration);
            if(!input_channels.float_channels.contains("param1") || !input_channels.float_channels.contains("param2")) {
                throw filter_config_exception("Unable to link input of math filter: channel mapping does not contain "
                                              "channel 'param1' or 'param2' of type 'double'.", own_type, own_id);
            }
            this->param1 = input_channels.float_channels.at("param1");
            this->param2 = input_channels.float_channels.at("param2");
        }

        virtual bool receive_update_from_gui(const std::string& key, const std::string& _value) override {
            MARK_UNUSED(key);
            MARK_UNUSED(_value);
            return false;
        }

        virtual void get_output_channels(channel_mapping& map, const std::string& name) override {
            map.float_channels[name + ":value"] = &output;
        }

        virtual void update() override {
            this->output = F(*param1, *param2);
        }

        virtual void scene_activated() override {}

    };


    using filter_logarithm = filter_math_single<std::log, filter_type::filter_logarithm>;
    using filter_exponential = filter_math_single<std::exp, filter_type::filter_exponential>;

    using filter_minimum = filter_math_dual<double, std::fmin, filter_type::filter_minimum>;
    using filter_maximum = filter_math_dual<double, std::fmax, filter_type::filter_maximum>;

    COMPILER_RESTORE("-Weffc++")

    template <typename T, filter_type own_type>
    class filter_multi_t_sum: public filter {
    private:
        COMPILER_SUPRESS("-Weffc++")
        std::vector<T*> params;
        COMPILER_RESTORE("-Weffc++")
        T output = 0;
    public:
        filter_multi_t_sum() : filter() {}
        virtual ~filter_multi_t_sum() {}

        virtual void setup_filter(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const channel_mapping& input_channels, const std::string& own_id) override {
            MARK_UNUSED(initial_parameters);
            long input_count;
            if(!configuration.contains("input_count")) [[unlikely]] {
                throw filter_config_exception("Paramter `item_count` missing.", own_type, own_id);
            } else {
                try {
                    input_count = std::stol(configuration.at("input_count"));
                    if (input_count < 0) [[unlikely]] {
                        throw filter_config_exception("`item_count` must not be negative.", own_type, own_id);
                    }
                } catch (const std::invalid_argument& e) {
                    throw filter_config_exception(std::string("Unable to decode `item_count` parameter: ") + e.what(),
                                                  own_type, own_id);
                }
                this->params.reserve(input_count);
            }
            for (auto i = 0; i < input_count; i++) {
                const auto key = std::to_string(i);
                auto& selected_map = get_const_channel_map(input_channels);
                if (!selected_map.contains(key)) [[unlikely]] {
                    throw filter_config_exception("Expected channel input map to contain key: " + key, own_type, own_id);
                }
                this->params.push_back(selected_map.at(key));
            }
        }

        virtual bool receive_update_from_gui(const std::string& key, const std::string& _value) override {
            MARK_UNUSED(key);
            MARK_UNUSED(_value);
            return false;
        }

        virtual void get_output_channels(channel_mapping& map, const std::string& name) override {
            get_channel_map(map)[name + ":value"] = &output;
        }

        virtual void update() override {
            this->output = 0;
            for (const auto& v_ptr : this->params) {
                if constexpr (std::is_same<T, double>::value) {
                    this->output += *v_ptr;
                } else {
                    this->output = (T) std::min((long) *v_ptr + (long) this->output,
                                                (long) std::numeric_limits<T>::max());
                }
            }
        }

        virtual void scene_activated() override {}
    private:
        constexpr std::map<std::string, T*>& get_channel_map(channel_mapping& input_channels) const {
            if constexpr (std::is_same<T, uint8_t>::value) {
                return input_channels.eight_bit_channels;
            } else if constexpr (std::is_same<T, uint16_t>::value) {
                return input_channels.sixteen_bit_channels;
            } else if constexpr (std::is_same<T, double>::value) {
                return input_channels.float_channels;
            } else {
                static_assert(std::is_same<T, uint8_t>::value || std::is_same<T, uint16_t>::value || std::is_same<T, double>::value, "This filter does not support colors.");
            }
        }

        constexpr const std::map<std::string, T*>& get_const_channel_map(const channel_mapping& input_channels) const {
            if constexpr (std::is_same<T, uint8_t>::value) {
                return input_channels.eight_bit_channels;
            } else if constexpr (std::is_same<T, uint16_t>::value) {
                return input_channels.sixteen_bit_channels;
            } else if constexpr (std::is_same<T, double>::value) {
                return input_channels.float_channels;
            } else {
                static_assert(std::is_same<T, uint8_t>::value || std::is_same<T, uint16_t>::value || std::is_same<T, double>::value, "This filter does not support colors.");
            }
        }
    };

    using filter_sum_8bit = filter_multi_t_sum<uint8_t, filter_type::filter_sum_8bit>;
    using filter_sum_16bit = filter_multi_t_sum<uint16_t, filter_type::filter_sum_16bit>;
    using filter_sum_float = filter_multi_t_sum<double, filter_type::filter_sum_float>;
}
