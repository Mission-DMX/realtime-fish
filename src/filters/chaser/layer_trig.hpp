#pragma once

#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <list>
#include <map>
#include <numbers>
#include <string>
#include <vector>

#include "cle_parameters.hpp"

namespace dmxfish::filters::chaserlayers {

    enum class trig_operations {
        SIN,
        COS,
        TAN
    };

    template <trig_operations op_type>
    class trig : public chaser_layer_executor {
    private:
        cle_number_parameter np_lowest_value, np_highest_value, np_phase;
    public:
        trig(std::list<std::string>& description, const std::map<std::string, uint16_t*>& number_inputs) {
            description.pop_front();
            this->np_lowest_value = cle_number_parameter(description.front(), number_inputs);
            description.pop_front();
            this->np_highest_value = cle_number_parameter(description.front(), number_inputs);
            description.pop_front();
            this->np_phase = cle_number_parameter(description.front(), number_inputs); // phase in degree
        }

        virtual void apply(const double elapsed_time, std::vector<dmxfish::dmx::pixel>& pixels, std::vector<uint16_t>& mask) override {
            const auto mask_size = mask.size();
            const auto phase = this->np_phase.get();
            const auto highest = this->np_highest_value.get();
            const auto lowest = this->np_lowest_value.get();
            for(auto i = 0; i < mask_size; i++) {
                if constexpr (op_type == trig_operations::SIN) {
                    mask[i] = (uint16_t) (std::sin(get_rad(i, phase, mask_size)) * (highest - lowest)) + lowest;
                } else if constexpr (op_type == trig_operations::COS) {
                    mask[i] = (uint16_t) (std::cos(get_rad(i, phase, mask_size)) * (highest - lowest)) + lowest;
                } else {
                    mask[i] = (uint16_t) (std::tan(get_rad(i, phase, mask_size)) * (highest - lowest)) + lowest;
                }
            }
        }

        virtual void reset() override {}
    private:
        [[nodiscard]] inline double get_rad(int val, uint16_t phase, size_t len) const {
            const double mod_val = ((double) val) / ((double) len) * 360.0 + phase;
            return (mod_val / 360.0) * std::numbers::pi;
        }

    };

}
