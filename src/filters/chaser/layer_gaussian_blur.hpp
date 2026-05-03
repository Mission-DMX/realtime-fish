#include "chaser_setup.hpp"

#include <algorithm>
#include <cmath>

#include "lib/macros.hpp"

namespace dmxfish::filters::chaserlayers {

    class gaussian_blur : public chaser_layer_executor {
    private:
        cle_number_parameter np_kernel_size;
        int last_kernel_size = -1;
        std::vector<double> kernel;
        std::vector<dmxfish::dmx::pixel> buffer;
    public:
        gaussian_blur(std::list<std::string>& description,
                const std::map<std::string, uint16_t*>& number_inputs) : kernel() {
                description.pop_front();
                this->np_kernel_size = cle_number_parameter(description.front(), number_inputs);
        }

        virtual void apply(const double elapsed_time, std::vector<dmxfish::dmx::pixel>& pixels, std::vector<uint16_t>& mask) override {
            MARK_UNUSED(elapsed_time);
            auto radius = this->np_kernel_size.get();
            if (radius > pixels.size() / 2) {
                radius = pixels.size() / 2;
            }
            if (radius < 1) {
                return;
            }
            if (radius != this->last_kernel_size) {
                this->make_kernel(radius);
                last_kernel_size = radius;
            }
            if (this->buffer.size() != pixels.size()) {
                this->buffer.resize(pixels.size());
            }
            const size_t pix_size = pixels.size();
            for(auto i = 0; i < pix_size; i++) {
                this->buffer[i] = pixels[i];
            }
            for (auto i = 0; i < pixels.size(); i++) {
                double r = 0, g = 0, b = 0;
                for (auto k = -radius; k <= radius; k++) {
                    size_t idx = static_cast<size_t>(std::clamp<size_t>(i + k, 0, pix_size - 1));
                    const auto weight = kernel[k + radius];
                    auto& src = buffer[idx];
                    r += weight * (src.getRed() / 65535.0);
                    g += weight * (src.getGreen() / 65535.0);
                    b += weight * (src.getBlue() / 65535.0);
                }
                dmxfish::dmx::pixel res_color((uint16_t) (r * 65535), (uint16_t) (b * 65535), (uint16_t) (b * 65535));
                pixels[i] = dmxfish::dmx::mix_color_interleaving(buffer[i], res_color, mask[i] / 65535.0);
            }
        }

        virtual void reset() override {}
        virtual void step() override {}
    private:
        inline void make_kernel(uint16_t radius) {
            const double sigma = radius / 3.0;
            const int size = 2 * radius + 1;
            auto& k = this->kernel;
            k.resize(size);
            const double twoSigmaSq = 2.0f * sigma * sigma;
            const double norm = 1.0 / std::sqrt(twoSigmaSq * static_cast<double>(M_PI));
            double sum = 0.0f;
            for (int i = -radius; i <= radius; ++i) {
                double x = static_cast<double>(i);
                double w = norm * std::exp(-(x * x) / twoSigmaSq);
                k[i + radius] = w;
                sum += w;
            }
            for (auto& v : k) {
                v /= sum;
            }
        }

    };

}
