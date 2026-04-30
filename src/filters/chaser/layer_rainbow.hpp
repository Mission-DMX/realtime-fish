#include "chaser_setup.hpp"

#include "lib/macros.hpp"

namespace dmxfish::filters::chaserlayers {

    class rainbow : public chaser_layer_executor {
    private:
        cle_color_parameter cp1, cp2;
	cle_number_parameter np;
    public:
        rainbow(std::list<std::string>& description,
                const std::map<std::string, dmxfish::dmx::pixel*>& color_inputs,
                const std::map<std::string, uint16_t*>& number_inputs) {
                description.pop_front();
                this->cp1 = cle_color_parameter(description.front(), color_inputs);
		description.pop_front();
		this->cp2 = cle_color_parameter(description.front(), color_inputs);
		description.pop_front();
		this->np = cle_number_parameter(description.front(), number_inputs);
        }

        virtual void apply(const double elapsed_time, std::vector<dmxfish::dmx::pixel>& pixels, std::vector<uint16_t>& mask) override {
            MARK_UNUSED(elapsed_time);
            const auto pix_count = pixels.size();
            int seg_count = this->np.get();
            if (seg_count < 1) {
                seg_count = 1;
            }
            seg_count = pixels.size() / seg_count;
            if (seg_count < 1) {
                seg_count = 1;
            }
            const double divisor = (seg_count == 1) ? 1 : seg_count - 1;
            for (auto i = 0; i < pix_count; i++) {
                auto dest_color = dmxfish::dmx::mix_color_interleaving(*(cp1.get()), *(cp2.get()), ((double) (i % seg_count)) / divisor);
                pixels[i] = dmxfish::dmx::mix_color_interleaving(pixels[i], dest_color, mask[i] / 65535.0);
            }
        }

        virtual void reset() override {}
        virtual void step() override {}

    };

}
