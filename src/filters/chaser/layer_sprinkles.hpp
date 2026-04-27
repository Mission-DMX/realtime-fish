#pragma once

#include <cstdlib>

namespace dmxfish::filters::chaserlayers {

    class sprinkles : public chaser_layer_executor {
    private:
        cle_number_parameter np_num_sprinkles, np_sprinkle_size, np_update_freq, np_mask_off, np_mask_on;
        long location_seed = 0;
        long remaining_time_of_locations = 0;
    public:
        sprinkles(std::list<std::string>& description, const std::map<std::string, uint16_t*>& number_inputs) {
            description.pop_front();
            this->np_num_sprinkles = cle_number_parameter(description.front(), number_inputs);
            description.pop_front();
            this->np_sprinkle_size = cle_number_parameter(description.front(), number_inputs);
            description.pop_front();
            this->np_update_freq = cle_number_parameter(description.front(), number_inputs);
            description.pop_front();
            this->np_mask_off = cle_number_parameter(description.front(), number_inputs);
            description.pop_front();
            this->np_mask_on = cle_number_parameter(description.front(), number_inputs);
            std::srand(std::time({}));
            this->location_seed = std::rand();
        }

        virtual void apply(const double elapsed_time, std::vector<dmxfish::dmx::pixel>& pixels, std::vector<uint16_t>& mask) override {
            this->remaining_time_of_locations -= (long) elapsed_time;
            std::srand(this->location_seed);
            if (const auto update_freq = this->np_update_freq.get(); this->remaining_time_of_locations < 0 && update_freq > 0) {
                this->remaining_time_of_locations = update_freq;
                this->location_seed = std::rand();
            }
            const auto num_sprinkles = this->np_num_sprinkles.get();
            const auto num_pixels = mask.size();
            const auto sprinkle_size = this->np_sprinkle_size.get();
            const auto mask_off = this->np_mask_off.get();
            for(auto i = 0; i < num_pixels; i++) {
                mask[i] = mask_off;
            }
            const auto half_sprinkle_size = sprinkle_size / 2;
            const auto mask_on = this->np_mask_on.get();
            for (auto i = 0; i < num_sprinkles; i++) {
                const auto center = (std::rand() % (num_pixels - sprinkle_size)) + (half_sprinkle_size);
                for(auto j = 0 - (half_sprinkle_size); j < half_sprinkle_size; j++) {
                    mask[j + center] = mask_on;
                }
            }
        }

        virtual void reset() override {}

    };

}
