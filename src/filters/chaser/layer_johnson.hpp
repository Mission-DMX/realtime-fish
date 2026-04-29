#pragma once

#include <cstdint>
#include <cstdlib>
#include <list>
#include <map>
#include <string>
#include <vector>

#include "cle_parameters.hpp"
#include "dmx/pixel.hpp"

namespace dmxfish::filters::chaserlayers {

    enum class direction {
        FWD,
        REV
    };

    template <direction op_type>
    class johnson : public chaser_layer_executor {
    private:
        cle_number_parameter np_update_speed, np_mask_setting;
        size_t progress = 0;
        long remaining_time = 0;
        bool is_buildup = true;
    public:
        johnson(std::list<std::string>& description, const std::map<std::string, uint16_t*>& number_inputs) {
            description.pop_front();
            this->np_update_speed = cle_number_parameter(description.front(), number_inputs);
            description.pop_front();
            this->np_mask_setting = cle_number_parameter(description.front(), number_inputs);
        }

        virtual void apply(const double elapsed_time, std::vector<dmxfish::dmx::pixel>& pixels, std::vector<uint16_t>& mask) override {
            const auto mask_size = mask.size();
            const auto update_time = this->np_update_speed.get();

            this->remaining_time -= (long) elapsed_time;
            if (update_time != 0 && this->remaining_time < 0) {
                this->remaining_time = update_time;
                if constexpr (op_type == direction::FWD) {
                    this->progress++;
                    if (progress > mask_size) {
                        this->progress = 0;
                        this->is_buildup = !(this->is_buildup);
                    }
                } else {
                    this->progress--;
                    if (progress > mask_size) {
                        this->progress = mask_size;
                        this->is_buildup = !(this->is_buildup);
                    }
                }
            }

            const auto mask_val = this->np_mask_setting.get();
            const auto prog = this->progress;
            for(auto i = 0; i < mask_size; i++) {
                if(this->is_buildup) {
                    if (i > prog) {
                        mask[i] = 0;
                    } else {
                        mask[i] = mask_val;
                    }
                } else {
                    if (i <= prog) {
                        mask[i] = 0;
                    } else {
                        mask[i] = mask_val;
                    }
                }
            }
        }

        virtual void reset() override {
            this->progress = 0;
            this->remaining_time = 0;
            this->is_buildup = true;
        }

    };

}

