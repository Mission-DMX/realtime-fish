#pragma once

#include <cstdint>
#include <cstdlib>
#include <list>
#include <map>
#include <string>
#include <vector>

#include "lib/macros.hpp"

#include "cle_parameters.hpp"
#include "operation_enum.hpp"

namespace dmxfish::filters::chaserlayers {

    template <mod_operation_type op_type>
    class mask_mod : public chaser_layer_executor {
    private:
        cle_number_parameter np_value, np_start, np_end;
    public:
        mask_mod(std::list<std::string>& description, const std::map<std::string, uint16_t*>& number_inputs) {
            description.pop_front();
            this->np_value = cle_number_parameter(description.front(), number_inputs);
            description.pop_front();
            this->np_start = cle_number_parameter(description.front(), number_inputs);
            description.pop_front();
            this->np_end = cle_number_parameter(description.front(), number_inputs);
        }

        virtual void apply(const double elapsed_time, std::vector<dmxfish::dmx::pixel>& pixels, std::vector<uint16_t>& mask) override {
            MARK_UNUSED(elapsed_time);
            MARK_UNUSED(pixels);
            const auto val = this->np_value.get();

            const auto mask_size = mask.size();

            const auto start_raw = this->np_start.get();
            const auto start = start_raw > 65534 ? mask_size : ((long) (start_raw * mask_size) / 65535);
            const auto end_raw = this->np_end.get();
            const auto end = end_raw > 65534 ? mask_size : ((long) (end_raw * mask_size) / 65535);
            for(long i = start; i < end; i++) {
                if constexpr (op_type == mod_operation_type::ADD) {
                    mask[i] = mask[i] + val;
                } else if constexpr (op_type == mod_operation_type::SUB) {
                    mask[i] = mask[i] - val;
                } else if constexpr (op_type == mod_operation_type::MUL) {
                    mask[i] = mask[i] * val;
                } else {
                    mask[i] = mask[i] / val;
                }
            }
        }

        virtual void reset() override {}
        virtual void step() override {}

    };

}
