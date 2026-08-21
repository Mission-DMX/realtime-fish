#pragma once

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <list>
#include <map>
#include <string>
#include <vector>

#include "lib/macros.hpp"

#include "cle_parameters.hpp"
#include "dmx/pixel.hpp"

namespace dmxfish::filters::chaserlayers {

    class dots : public chaser_layer_executor {
    private:
        cle_number_parameter np_num_dots, np_dots_size, np_update_freq, np_mask_off, np_mask_on;
        long location_offset = 0;
        long remaining_time_of_locations = 0;
    public:
        dots(std::list<std::string>& description, const std::map<std::string, uint16_t*>& number_inputs) {
            description.pop_front();
            this->np_num_dots = cle_number_parameter(description.front(), number_inputs);
            description.pop_front();
            this->np_dots_size = cle_number_parameter(description.front(), number_inputs);
            description.pop_front();
            this->np_update_freq = cle_number_parameter(description.front(), number_inputs);
            description.pop_front();
            this->np_mask_off = cle_number_parameter(description.front(), number_inputs);
            description.pop_front();
            this->np_mask_on = cle_number_parameter(description.front(), number_inputs);
        }

        virtual void apply(const double elapsed_time, std::vector<dmxfish::dmx::pixel>& pixels, std::vector<uint16_t>& mask) override {
            MARK_UNUSED(pixels);
            this->remaining_time_of_locations -= (long) elapsed_time;
            if (const auto update_freq = this->np_update_freq.get(); this->remaining_time_of_locations < 0 && update_freq > 0) {
                this->remaining_time_of_locations = update_freq;
                this->location_offset++;
            }

            const auto num_dots = std::max((uint16_t) 1, this->np_num_dots.get());
            const auto num_pixels = mask.size();
            const auto mask_off = this->np_mask_off.get();
            const auto mask_on = this->np_mask_on.get();
            const auto dot_size = this->np_dots_size.get();

            const auto seg_len = num_pixels / num_dots;
            const auto on_len = seg_len - dot_size;

            for(auto i = 0; i < num_pixels; i++) {
                const auto pos_in_seg = (i + this->location_offset) % seg_len;
                mask[i] = (pos_in_seg < on_len) ? mask_on : mask_off;
            }
        }

        virtual void reset() override {
            this->location_offset = 0;
        }

        virtual void step() override {
            this->location_offset++;
        }

    };

}
