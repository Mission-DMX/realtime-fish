#include "chaser_setup.hpp"

#include "lib/macros.hpp"

namespace dmxfish::filters::chaserlayers {

    class plain_color : public chaser_layer_executor {
    private:
        cle_color_parameter cp1;
    public:
        plain_color(std::list<std::string>& description,
                const std::map<std::string, dmxfish::dmx::pixel*>& color_inputs,
                const std::map<std::string, uint16_t*>& number_inputs) {
                description.pop_front();
                this->cp1 = cle_color_parameter(description.front(), color_inputs);
        }

        virtual void apply(const double elapsed_time, std::vector<dmxfish::dmx::pixel>& pixels, std::vector<uint16_t>& mask) override {
            MARK_UNUSED(elapsed_time);
            for (auto i = 0; i < pixels.size(); i++) {
                pixels[i] = dmxfish::dmx::mix_color_interleaving(pixels[i], *(this->cp1.get()), mask[i] / 65535.0);
            }
        }

        virtual void reset() override {}

        virtual void step() override {}

    };

}
