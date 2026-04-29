#include "chaser_setup.hpp"

#include "lib/macros.hpp"

namespace dmxfish::filters::chaserlayers {

    class colormix : public chaser_layer_executor {
    private:
        cle_color_parameter cp1, cp2;
    public:
        colormix(std::list<std::string>& description,
                const std::map<std::string, dmxfish::dmx::pixel*>& color_inputs) {
                description.pop_front();
                this->cp1 = cle_color_parameter(description.front(), color_inputs);
                description.pop_front();
                this->cp2 = cle_color_parameter(description.front(), color_inputs);
        }

        virtual void apply(const double elapsed_time, std::vector<dmxfish::dmx::pixel>& pixels, std::vector<uint16_t>& mask) override {
            for (auto i = 0; i < pixels.size(); i++) {
                const auto res_color = dmxfish::dmx::mix_color_interleaving(*(this->cp1.get()), *(this->cp2.get()), 0.5);
                pixels[i] = dmxfish::dmx::mix_color_interleaving(pixels[i], res_color, mask[i] / 65535.0);
            }
        }

        virtual void reset() override {}

    };

}
