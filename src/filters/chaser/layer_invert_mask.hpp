#pragma once

#include <cstdint>
#include <cstdlib>
#include <list>
#include <map>
#include <string>
#include <vector>

#include "lib/macros.hpp"

#include "cle_parameters.hpp"

namespace dmxfish::filters::chaserlayers {

    class invert_mask : public chaser_layer_executor {
    public:
        invert_mask() {}

        virtual void apply(const double elapsed_time, std::vector<dmxfish::dmx::pixel>& pixels, std::vector<uint16_t>& mask) override {
            MARK_UNUSED(elapsed_time);
            MARK_UNUSED(pixels);
            const auto mask_size = mask.size();
            for(auto i = 0; i < mask_size; i++) {
                mask[i] = 65535 - mask[i];
            }
        }

        virtual void reset() override {}

        virtual void step() override {}

    };

}
