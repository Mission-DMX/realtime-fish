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

    class mask_strobe : public chaser_layer_executor {
    private:
        cle_number_parameter np_bpm;
        int remaining_time = 0;
        bool currently_on = true;
    public:
        mask_strobe(std::list<std::string>& description, const std::map<std::string, uint16_t*>& number_inputs) {
            description.pop_front();
            this->np_bpm = cle_number_parameter(description.front(), number_inputs);
        }

        virtual void apply(const double elapsed_time, std::vector<dmxfish::dmx::pixel>& pixels, std::vector<uint16_t>& mask) override {
            MARK_UNUSED(pixels);
            this->remaining_time -= (int) elapsed_time;
            if (this->remaining_time <= 0) {
                const auto bpm = this->np_bpm.get();
                this->remaining_time = (bpm > 0) ? ((bpm <= 750) ? (int) (60000.0 / (bpm * 2.0)) : 0) : INT32_MAX;
                this->currently_on = !(this->currently_on);
            }

            const auto mask_size = mask.size();

            if(!this->currently_on) {
                for(auto i = 0; i < mask_size; i++) {
                    mask[i] = 0;
                }
            }
        }

        virtual void reset() override {
            this->remaining_time = 0;
            this->currently_on = false;
        }

        virtual void step() override {
            this->remaining_time = 0;
        }

    };

}
