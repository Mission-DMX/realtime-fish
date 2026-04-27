#pragma once

#include <cstdint>
#include <cstdlib>
#include <list>
#include <map>
#include <string>
#include <vector>

#include "cle_parameters.hpp"
#include "dmx/pixel.hpp"

namespace dmxfish::filters::chaserlayers {

    class scale : public chaser_layer_executor {
    private:
        cle_number_parameter np_scale_start, np_scale_end, np_mask_off, np_mask_on;
    public:
        scale(std::list<std::string>& description, const std::map<std::string, uint16_t*>& number_inputs) {
            description.pop_front();
            this->np_scale_start = cle_number_parameter(description.front(), number_inputs);
            description.pop_front();
            this->np_scale_end = cle_number_parameter(description.front(), number_inputs);
            description.pop_front();
            this->np_mask_off = cle_number_parameter(description.front(), number_inputs);
            description.pop_front();
            this->np_mask_on = cle_number_parameter(description.front(), number_inputs);
        }

        virtual void apply(const double elapsed_time, std::vector<dmxfish::dmx::pixel>& pixels, std::vector<uint16_t>& mask) override {
            const auto scale_start = this->np_scale_start.get();
            const auto num_pixels = mask.size();
            const auto mask_off = this->np_mask_off.get();
            const auto mask_on = this->np_mask_on.get();
            const auto scale_end = this->np_scale_end.get();

            for(auto i = 0; i < num_pixels; i++) {
                if (i < scale_start) {
                    mask[i] = mask_on;
                } else if (i < scale_end) {
                    mask[i] = ((mask_off - mask_on) / (scale_end - scale_start)) * (i - scale_start) + mask_on;
                } else {
                    mask[i] = mask_off;
                }
            }
        }

        virtual void reset() override {}

    };

}

