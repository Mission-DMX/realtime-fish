#pragma once

/*
 * This filter gates a brightness input through a square-wave strobe generator.
 * The strobe frequency is driven by an 8-bit `frequency` channel and its duty
 * cycle by an 8-bit `duty_cycle` channel. The brightness channel data type is
 * selected via the template parameter and determines the output data type.
 */

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <map>
#include <string>
#include <type_traits>

#include "filters/filter.hpp"
#include "lib/macros.hpp"

namespace dmxfish::filters {

    COMPILER_SUPRESS("-Weffc++")

    template <typename T, filter_type own_type>
    class filter_series_strobe_generator : public filter {
    private:
        T* brightness_input = nullptr;
        uint8_t* frequency_input = nullptr;
        uint8_t* duty_cycle_input = nullptr;
        T output;

        // Period at frequency_input == 1, expressed in 25 ms update ticks.
        double slowest_period_ticks = 0.0;
        unsigned int cycle_tick = 0;

        // Full-cycle frequency at the strobe maximum: a 2-tick cycle on a 40 Hz
        // update loop gives 20 Hz full cycles.
        static constexpr double update_rate_hz = 40.0;
        static constexpr double max_full_cycle_hz = 20.0;

    public:
        filter_series_strobe_generator() : filter(), output{} {
            static_assert(std::is_same<T, uint8_t>::value || std::is_same<T, uint16_t>::value || std::is_same<T, double>::value,
                          "filter_series_strobe_generator only supports uint8_t, uint16_t or double brightness channels.");
        }
        virtual ~filter_series_strobe_generator() {}

        virtual void setup_filter(const std::map<std::string, std::string>& configuration,
                                  const std::map<std::string, std::string>& initial_parameters,
                                  const channel_mapping& input_channels,
                                  const std::string& own_id) override {
            MARK_UNUSED(initial_parameters);

            if (!configuration.contains("slowest_period_ms")) {
                throw filter_config_exception("Unable to setup series strobe generator: configuration does not contain "
                                              "a value for 'slowest_period_ms'", own_type, own_id);
            }
            double slowest_period_ms = 0.0;
            try {
                slowest_period_ms = std::stod(configuration.at("slowest_period_ms"));
            } catch (const std::invalid_argument& ex) {
                MARK_UNUSED(ex);
                throw filter_config_exception("Unable to setup series strobe generator: could not parse "
                                              "'slowest_period_ms' as a floating point number", own_type, own_id);
            } catch (const std::out_of_range& ex) {
                MARK_UNUSED(ex);
                throw filter_config_exception("Unable to setup series strobe generator: 'slowest_period_ms' is out of "
                                              "range", own_type, own_id);
            }
            // The slowest configured period must be at least the fastest achievable
            // one (2 update ticks = 50 ms); anything shorter would mean "slowest ==
            // fastest" and reduce to the max-frequency case.
            if (slowest_period_ms < 1000.0 / max_full_cycle_hz) {
                throw filter_config_exception("Unable to setup series strobe generator: 'slowest_period_ms' must be at "
                                              "least 50 ms (2 update ticks at 40 Hz)", own_type, own_id);
            }
            this->slowest_period_ticks = slowest_period_ms * update_rate_hz / 1000.0;

            if (!input_channels.eight_bit_channels.contains("frequency")) {
                throw filter_config_exception("Unable to link input of series strobe generator: channel mapping does "
                                              "not contain channel 'frequency' of type 'uint8_t'.", own_type, own_id);
            }
            this->frequency_input = input_channels.eight_bit_channels.at("frequency");

            if (!input_channels.eight_bit_channels.contains("duty_cycle")) {
                throw filter_config_exception("Unable to link input of series strobe generator: channel mapping does "
                                              "not contain channel 'duty_cycle' of type 'uint8_t'.", own_type, own_id);
            }
            this->duty_cycle_input = input_channels.eight_bit_channels.at("duty_cycle");

            if constexpr (std::is_same<T, uint8_t>::value) {
                if (!input_channels.eight_bit_channels.contains("brightness")) {
                    throw filter_config_exception("Unable to link input of series strobe generator: channel mapping "
                                                  "does not contain channel 'brightness' of type 'uint8_t'.", own_type,
                                                  own_id);
                }
                this->brightness_input = input_channels.eight_bit_channels.at("brightness");
            } else if constexpr (std::is_same<T, uint16_t>::value) {
                if (!input_channels.sixteen_bit_channels.contains("brightness")) {
                    throw filter_config_exception("Unable to link input of series strobe generator: channel mapping "
                                                  "does not contain channel 'brightness' of type 'uint16_t'.", own_type,
                                                  own_id);
                }
                this->brightness_input = input_channels.sixteen_bit_channels.at("brightness");
            } else {
                if (!input_channels.float_channels.contains("brightness")) {
                    throw filter_config_exception("Unable to link input of series strobe generator: channel mapping "
                                                  "does not contain channel 'brightness' of type 'double'.", own_type,
                                                  own_id);
                }
                this->brightness_input = input_channels.float_channels.at("brightness");
            }
        }

        virtual void get_output_channels(channel_mapping& map, const std::string& name) override {
            if constexpr (std::is_same<T, uint8_t>::value) {
                map.eight_bit_channels[name + ":value"] = &this->output;
            } else if constexpr (std::is_same<T, uint16_t>::value) {
                map.sixteen_bit_channels[name + ":value"] = &this->output;
            } else {
                map.float_channels[name + ":value"] = &this->output;
            }
        }

        virtual void update() override {
            const uint8_t freq = *this->frequency_input;

            if (freq == 0) {
                this->output = *this->brightness_input;
                this->cycle_tick = 0;
                return;
            }

            const double slowest_freq_hz = update_rate_hz / this->slowest_period_ticks;
            const double t = static_cast<double>(freq - 1) / 254.0;
            const double current_hz = slowest_freq_hz + t * (max_full_cycle_hz - slowest_freq_hz);
            const unsigned int period_ticks = std::max(2u,
                static_cast<unsigned int>(std::round(update_rate_hz / current_hz)));

            const uint8_t duty = *this->duty_cycle_input;
            unsigned int on_ticks;
            if (period_ticks <= 2) {
                on_ticks = 1;
            } else {
                on_ticks = 1 + static_cast<unsigned int>(std::round((duty / 255.0) * (period_ticks - 2)));
            }

            if (this->cycle_tick < on_ticks) {
                this->output = *this->brightness_input;
            } else {
                this->output = T{};
            }

            this->cycle_tick = (this->cycle_tick + 1) % period_ticks;
        }

        virtual void scene_activated() override {
            this->cycle_tick = 0;
            this->output = T{};
        }

        virtual bool receive_update_from_gui(const std::string& key, const std::string& value) override {
            MARK_UNUSED(key);
            MARK_UNUSED(value);
            return false;
        }
    };

    using filter_series_strobe_generator_8bit  = filter_series_strobe_generator<uint8_t,  filter_type::filter_series_strobe_generator_8bit>;
    using filter_series_strobe_generator_16bit = filter_series_strobe_generator<uint16_t, filter_type::filter_series_strobe_generator_16bit>;
    using filter_series_strobe_generator_float = filter_series_strobe_generator<double,   filter_type::filter_series_strobe_generator_float>;

    COMPILER_RESTORE("-Weffc++")

}
