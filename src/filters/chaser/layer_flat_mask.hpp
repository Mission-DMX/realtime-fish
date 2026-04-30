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

    class flat_mask : public chaser_layer_executor {
    private:
        cle_number_parameter np_value;
    public:
        flat_mask(std::list<std::string>& description, const std::map<std::string, uint16_t*>& number_inputs) {
            description.pop_front();
            this->np_value = cle_number_parameter(description.front(), number_inputs);
        }

        virtual void apply(const double elapsed_time, std::vector<dmxfish::dmx::pixel>& pixels, std::vector<uint16_t>& mask) override {
            MARK_UNUSED(elapsed_time);
            MARK_UNUSED(pixels);
            const auto val = this->np_value.get();
	    const auto mask_size = mask.size();
            for(auto i = 0; i < mask_size; i++) {
                mask[i] = val;
            }
        }

        virtual void reset() override {}

        virtual void step() override {}

    };

}
