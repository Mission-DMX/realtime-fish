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

    class close_to_center : public chaser_layer_executor {
    private:
        cle_number_parameter np_update_time, np_decay, np_mask_application_intensity;
        size_t current_step = 0;
        int remaining_time = 0;
    public:
        close_to_center(std::list<std::string>& description, const std::map<std::string, uint16_t*>& number_inputs) {
            description.pop_front();
            this->np_update_time = cle_number_parameter(description.front(), number_inputs);
            description.pop_front();
            this->np_decay = cle_number_parameter(description.front(), number_inputs);
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

            if(this->current_step >= mask_size / 2) {
                this->current_step = 0;
            }

            const auto _step = this->current_step;
            const double _mult_factor = 1.0 - (((double) this->np_decay.get()) / 65535.0);
            const auto intensity = this->np_mask_application_intensity.get();

            for(auto i = 0; i < mask_size; i++) {
                const auto distance_to_wall = std::abs((int) _step - i);
                const bool is_active = i < mask_size / 2 ? i < _step : (i - mask_size / 2) > _step;
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

}
