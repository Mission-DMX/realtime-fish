#include "chaser_setup.hpp"

#include "lib/macros.hpp"

namespace dmxfish::filters::chaserlayers {

    class invert_color : public chaser_layer_executor {
    public:
        invert_color() {}

        virtual void apply(const double elapsed_time, std::vector<dmxfish::dmx::pixel>& pixels, std::vector<uint16_t>& mask) override {
            MARK_UNUSED(elapsed_time);
            for (auto i = 0; i < pixels.size(); i++) {
                auto& c = pixels[i];
                dmxfish::dmx::pixel inverted((uint16_t) (65535 - c.getRed()), (uint16_t) (65535 - c.getGreen()), (uint16_t) (65535 - c.getBlue()));
                c = dmxfish::dmx::mix_color_interleaving(pixels[i], inverted, mask[i] / 65535.0);
            }
        }

        virtual void reset() override {}

        virtual void step() override {}

    };

}
