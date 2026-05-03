#pragma once

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cmath>
#include <list>
#include <map>
#include <numbers>
#include <string>
#include <vector>

#include "lib/macros.hpp"

#include "cle_parameters.hpp"

namespace dmxfish::filters::chaserlayers {

    class gaussian_curve : public chaser_layer_executor {
    private:
        cle_number_parameter np_position, np_width, np_height;
    public:
        gaussian_curve(std::list<std::string>& description, const std::map<std::string, uint16_t*>& number_inputs) {
            description.pop_front();
            this->np_position = cle_number_parameter(description.front(), number_inputs);
            description.pop_front();
            this->np_width = cle_number_parameter(description.front(), number_inputs);
            description.pop_front();
            this->np_height = cle_number_parameter(description.front(), number_inputs);
        }

        virtual void apply(const double elapsed_time, std::vector<dmxfish::dmx::pixel>& pixels, std::vector<uint16_t>& mask) override {
            MARK_UNUSED(elapsed_time);
            MARK_UNUSED(pixels);
            const auto mask_size = mask.size();
            const double mu = (((double) this->np_position.get()) / 65535.0) * (double) mask_size;
            const double sigma_s = 2.0 * (((double) this->np_width.get()) / ((double) mask_size)) * (mask_size / 2.0);
            const double factor = (2.0/sigma_s) * ((double) (this->np_height.get()) / 65535.0);
            const double prefix = (1 / (std::sqrt(0.5 * sigma_s) * std::sqrt(2 * std::numbers::pi)));
	    
            for(auto i = 0; i < mask_size; i++) {
                const double val = prefix * std::exp(0 - ((i - mu) * (i - mu)) / sigma_s);
                mask[i] = std::clamp((int) (mask[i] + (val * 65535.0)), 0, 65535);
            }
        }

        virtual void reset() override {}

        virtual void step() override {}

    };

}
