#pragma once

#include <cstdint>
#include <cstdlib>
#include <list>
#include <map>
#include <string>
#include <vector>

#include "lib/macros.hpp"

#include "cle_parameters.hpp"

namespace dmxfish::filters::chaserlayers {

    class color_shift : public chaser_layer_executor {
    private:
        cle_number_parameter np_time;
        unsigned int current_shift = 0;
        long remaining_time = 0;
        std::vector<dmxfish::dmx::pixel> pixel_copy;
    public:
        color_shift(std::list<std::string>& description, const std::map<std::string, uint16_t*>& number_inputs) {
            description.pop_front();
            this->np_time = cle_number_parameter(description.front(), number_inputs);
        }

        virtual void apply(const double elapsed_time, std::vector<dmxfish::dmx::pixel>& pixels, std::vector<uint16_t>& mask) override {
            this->remaining_time -= (long) elapsed_time;
            const auto mask_size = mask.size();
            if (this->remaining_time <= 0) {
                this->remaining_time = this->np_time.get();
                this->current_shift = (this->current_shift + 1) % mask_size;
            }

            if(this->pixel_copy.size() != mask_size) {
                this->pixel_copy.resize(mask_size);
            }

            for(auto i = 0; i < mask_size; i++) {
                pixel_copy[i] = pixels[i];
            }
            for(auto i = 0; i < mask_size; i++) {
                pixels[i] = dmxfish::dmx::mix_color_interleaving(pixels[i], pixel_copy[(i + this->current_shift) % pixel_copy.size()], mask[i] / 65535.0);
            }
        }

        virtual void reset() override {
            this->current_shift = 0;
        }

        virtual void step() override {
            this->remaining_time = 0;
        }

    };

}
