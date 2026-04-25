#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "filters/filter.hpp"
#include "lib/macros.hpp"

#include "chaser_setup.hpp"

namespace dmxfish::filters {

        class  filter_color_chaser : public filter {
        private:
		friend class chaser_setup;

                COMPILER_SUPRESS("-Weffc++")
                std::map<std::string, dmxfish::dmx::pixel*> color_parameter_inputs;
                std::map<std::string, uint16_t*> number_parameter_inputs;
                double* time_input = nullptr;
                double* timescale_input = nullptr;
                COMPILER_RESTORE("-Weffc++")

                std::vector<dmxfish::dmx::pixel> pixels;
                std::vector<uint16_t> mask;
                std::unique_ptr<chaser_setup> setup = nullptr;
                std::string own_id;
        public:
                filter_color_chaser();
		virtual void pre_setup(
                                const std::map<std::string, std::string>& configuration,
                                const std::map<std::string, std::string>& initial_parameters,
                                const std::string& own_id
                ) override;
                virtual void setup_filter(
                                const std::map<std::string, std::string>& configuration,
                                const std::map<std::string, std::string>& initial_parameters,
                                const channel_mapping& input_channels,
				const std::string& own_id
                ) override;
                virtual void get_output_channels(channel_mapping& map, const std::string& name) override;
                virtual void update() override;
                virtual void scene_activated() override;
		virtual bool receive_update_from_gui(const std::string& key, const std::string& _value) override;
        };
}
