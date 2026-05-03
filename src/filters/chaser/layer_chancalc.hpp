#include "chaser_setup.hpp"

#include <algorithm>
#include <map>

#include "color_channel_enum.hpp"
#include "operation_enum.hpp"
#include "lib/macros.hpp"

namespace dmxfish::filters::chaserlayers {

    template <color_channel_target target_channel, mod_operation_type op_type>
    class chancalc : public chaser_layer_executor {
    private:
        cle_number_parameter np_target_value;
    public:
        chancalc(std::list<std::string>& description, const std::map<std::string, uint16_t*>& number_inputs) {
                description.pop_front();
                this->np_target_value = cle_number_parameter(description.front(), number_inputs);
        }

        virtual void apply(const double elapsed_time, std::vector<dmxfish::dmx::pixel>& pixels, std::vector<uint16_t>& mask) override {
            MARK_UNUSED(elapsed_time);
            MARK_UNUSED(mask);
            for (auto i = 0; i < pixels.size(); i++) {
		int val;
                if constexpr (target_channel == color_channel_target::R) {
                    val = pixels[i].getRed();
                } else if constexpr (target_channel == color_channel_target::G) {
                    val = pixels[i].getGreen();
                } else if constexpr (target_channel == color_channel_target::B) {
                    val = pixels[i].getBlue();
                } else if constexpr (target_channel == color_channel_target::H) {
                    val = pixels[i].getHue();
                } else if constexpr (target_channel == color_channel_target::S) {
                    val = pixels[i].getSaturation() * 65535.0;
                } else {
                    val = pixels[i].getIluminance() * 65535.0;
                }
		if constexpr (op_type == mod_operation_type::ADD) {
		    val += this->np_target_value.get();
		} else if constexpr (op_type == mod_operation_type::SUB) {
		    val -= this->np_target_value.get();
		} else if constexpr (op_type == mod_operation_type::MUL) {
		    val *= this->np_target_value.get();
		} else {
		    val /= this->np_target_value.get();
		}
		if constexpr (target_channel == color_channel_target::R) {
                    pixels[i].setRed(std::max(std::min(val, 65535), 0));
                } else if constexpr (target_channel == color_channel_target::G) {
                    pixels[i].setGreen(std::max(std::min(val, 65535), 0));
                } else if constexpr (target_channel == color_channel_target::B) {
                    pixels[i].setBlue(std::max(std::min(val, 65535), 0));
                } else if constexpr (target_channel == color_channel_target::H) {
                    pixels[i].setHue(((double) std::max(std::min(val, 65535), 0)) / 65535.0);
                } else if constexpr (target_channel == color_channel_target::S) {
                    pixels[i].setSaturation(((double) std::max(std::min(val, 65535), 0)) / 65535.0);
                } else {
                    pixels[i].setIluminance(((double) std::max(std::min(val, 65535), 0)) / 65535.0);
                }
            }
        }

        virtual void reset() override {}
        virtual void step() override {}

    };

}
