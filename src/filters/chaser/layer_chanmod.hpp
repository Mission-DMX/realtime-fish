#include "chaser_setup.hpp"

#include <map>

#include "color_channel_enum.hpp"
#include "lib/macros.hpp"

namespace dmxfish::filters::chaserlayers {

    template <color_channel_target target_channel>
    class chanmod : public chaser_layer_executor {
    private:
        cle_number_parameter np_target_value;
    public:
        chanmod(std::list<std::string>& description, const std::map<std::string, uint16_t*>& number_inputs) {
                description.pop_front();
                this->np_target_value = cle_number_parameter(description.front(), number_inputs);
        }

        virtual void apply(const double elapsed_time, std::vector<dmxfish::dmx::pixel>& pixels, std::vector<uint16_t>& mask) override {
            MARK_UNUSED(elapsed_time);
            MARK_UNUSED(mask);
            for (auto i = 0; i < pixels.size(); i++) {
                if constexpr (target_channel == color_channel_target::R) {
                    pixels[i].setRed(this->np_target_value.get());
                } else if constexpr (target_channel == color_channel_target::G) {
                    pixels[i].setGreen(this->np_target_value.get());
                } else if constexpr (target_channel == color_channel_target::B) {
                    pixels[i].setBlue(this->np_target_value.get());
                } else if constexpr (target_channel == color_channel_target::H) {
                    pixels[i].setHue(((double) this->np_target_value.get()) / 65535.0);
                } else if constexpr (target_channel == color_channel_target::S) {
                    pixels[i].setSaturation(((double) this->np_target_value.get()) / 65535.0);
                } else {
                    pixels[i].setIluminance(((double) this->np_target_value.get()) / 65535.0);
                }
            }
        }

        virtual void reset() override {}
        virtual void step() override {}

    };

}
