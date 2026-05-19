#include "chaser_setup.hpp"

#include <algorithm>
#include <cstdlib>
#include <ctime>

#include "lib/macros.hpp"

namespace dmxfish::filters::chaserlayers {

    class randomcolor : public chaser_layer_executor {
    private:
        cle_number_parameter np_number_of_colors, np_update_time;
        unsigned int color_seed = 0;
        int remaining_time = 0;
    public:
        randomcolor(std::list<std::string>& description,
                const std::map<std::string, uint16_t*>& number_inputs) {
                description.pop_front();
                this->np_number_of_colors = cle_number_parameter(description.front(), number_inputs);
                description.pop_front();
                this->np_update_time = cle_number_parameter(description.front(), number_inputs);
                std::srand(std::time({}));
                this->color_seed = std::rand();
        }

        virtual void apply(const double elapsed_time, std::vector<dmxfish::dmx::pixel>& pixels, std::vector<uint16_t>& mask) override {
            this->remaining_time -= elapsed_time;
            if (this->remaining_time < 0) {
                this->remaining_time = this->np_update_time.get();
                this->step();
            }
            const auto pixels_per_segment = pixels.size() / std::max((uint16_t) 1, this->np_number_of_colors.get());
            std::srand(this->color_seed);
            uint16_t r = 0;
            uint16_t g = 0;
            uint16_t b = 0;
            for (auto i = 0; i < pixels.size(); i++) {
                if (i % pixels_per_segment == 0) {
                    r = std::rand() % 65535;
                    g = std::rand() % 65535;
                    b = std::rand() % 65535;
                }
                const auto res_color = dmxfish::dmx::pixel(r, g, b);
                pixels[i] = dmxfish::dmx::mix_color_interleaving(pixels[i], res_color, mask[i] / 65535.0);
            }
        }

        virtual void reset() override {}
        
        virtual void step() override {
            std::srand(this->color_seed);
            this->color_seed = std::rand();
        }

    };

}
