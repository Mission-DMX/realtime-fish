#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <list>
#include <map>
#include <string>
#include <vector>

#include "lib/macros.hpp"

#include "cle_parameters.hpp"
#include "operation_enum.hpp"

namespace dmxfish::filters::chaserlayers {

    template <bool is_forward>
    class segwave : public chaser_layer_executor {
    private:
        cle_number_parameter np_update_time, np_decay, np_num_waves, np_mask_application_intensity;
        size_t current_step = 0;
        int remaining_time = 0;
    public:
        segwave(std::list<std::string>& description, const std::map<std::string, uint16_t*>& number_inputs) {
            description.pop_front();
            this->np_update_time = cle_number_parameter(description.front(), number_inputs);
            description.pop_front();
            this->np_decay = cle_number_parameter(description.front(), number_inputs);
            description.pop_front();
            this->np_num_waves = cle_number_parameter(description.front(), number_inputs);
            description.pop_front();
            this->np_mask_application_intensity = cle_number_parameter(description.front(), number_inputs);
        }

        virtual void apply(const double elapsed_time, std::vector<dmxfish::dmx::pixel>& pixels, std::vector<uint16_t>& mask) override {
            MARK_UNUSED(pixels);

            remaining_time -= (int) elapsed_time;
            if (remaining_time < 0) {
                this->current_step++;
                this->remaining_time = this->np_update_time.get();
            }

            const auto mask_size = mask.size();
            const auto _step = this->current_step;

            const double _mult_factor = 1.0 - (((double) this->np_decay.get()) / 65535.0);
            const auto intensity = this->np_mask_application_intensity.get();
            constexpr auto direction_add = is_forward ? 1 : -1;
            const auto wave_div = mask_size / this->np_num_waves.get();

            for(auto i = is_forward ? 0 : mask_size - 1; is_forward ? i < mask_size : i >= 0; i += direction_add) {
                const auto distance_to_wall = (int) (_step % wave_div) - (i % wave_div);

                const bool is_active = distance_to_wall > 0;

                const auto val = is_active ? std::pow(_mult_factor, distance_to_wall) * intensity : 0;
                mask[i] += (uint16_t) val;
            }
        }

        virtual void reset() override {
            this->current_step = 0;
            this->remaining_time = 0;
        }

        virtual void step() override {
            this->current_step++;
        }
    };

    template <bool is_forward>
    class wave : public chaser_layer_executor {
    private:
        cle_number_parameter np_update_time, np_decay, np_num_waves, np_mask_application_intensity;
        size_t current_step = 0;
        int remaining_time = 0;
    public:
        wave(std::list<std::string>& description, const std::map<std::string, uint16_t*>& number_inputs) {
            description.pop_front();
            this->np_update_time = cle_number_parameter(description.front(), number_inputs);
            description.pop_front();
            this->np_decay = cle_number_parameter(description.front(), number_inputs);
            description.pop_front();
            this->np_num_waves = cle_number_parameter(description.front(), number_inputs);
            description.pop_front();
            this->np_mask_application_intensity = cle_number_parameter(description.front(), number_inputs);
        }

        virtual void apply(const double elapsed_time, std::vector<dmxfish::dmx::pixel>& pixels, std::vector<uint16_t>& mask) override {
            MARK_UNUSED(pixels);

            remaining_time -= (int) elapsed_time;
            if (remaining_time < 0) {
                this->current_step++;
                this->remaining_time = this->np_update_time.get();
            }

            const auto mask_size = mask.size();
            const auto _step = this->current_step;

            const double _mult_factor = 1.0 - (((double) this->np_decay.get()) / 65535.0);
            const auto init_intensity = this->np_mask_application_intensity.get();
            constexpr auto direction_add = is_forward ? -1 : 1;
            const auto wave_div = mask_size / this->np_num_waves.get();

            double intensity = 0;

            for(long i = is_forward ? mask_size + wave_div : 0 - wave_div; is_forward ? i > 0 : i < mask_size; i += direction_add) {
                if ((i + _step) % wave_div == 0) {
                    intensity = init_intensity;
                } else {
                    intensity *= _mult_factor;
                }
                if (i >= mask_size || i < 0) {
                    continue;
                }

                mask[i] += (uint16_t) intensity;
            }
        }

        virtual void reset() override {
            this->current_step = 0;
            this->remaining_time = 0;
        }

        virtual void step() override {
            this->current_step++;
        }
    };

}
