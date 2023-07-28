#pragma once

#include <stdexcept>
#include <string>
#include <vector>

#include "filters/filter.hpp"
#include "io/universe_sender.hpp"
#include "lib/macros.hpp"

namespace dmxfish::filters {

    COMPILER_SUPRESS("-Weffc++")
    class filter_universe_output : public filter {
    private:
        struct output_link {
            uint16_t universe_channel;
            uint8_t* input_channel;

            output_link(uint16_t _universe_channel, uint8_t* _input_channel) : universe_channel(_universe_channel), input_channel(_input_channel) {}
        };
        int universe_id = 0;
        std::vector<output_link> mapping;
    public:
        filter_universe_output() : filter(), mapping{} {}
        virtual ~filter_universe_output() {}

        virtual void setup_filter(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const channel_mapping& input_channels) override {
            MARK_UNUSED(initial_parameters);
            this->mapping.reserve(configuration.size() - 1);
            if(!configuration.contains("universe")){
                throw filter_config_exception("A universe id must be set in order to let the universe output filter do its work.");
            } else {
                this->universe_id = std::stoi(configuration.at("universe"));
                if(dmxfish::io::get_universe(this->universe_id) == nullptr) {
                    throw filter_config_exception("The configured universe id does not seam to match a universe.");
                }
            }
            for(const auto& [uchannel, ichannel] : configuration) {
                if(uchannel != "universe") {
                    try {
                        if(!input_channels.eight_bit_channels.contains(ichannel)) {
                            throw filter_config_exception("Failed to configure output filter for universe " + std::to_string(this->universe_id) + ": input channel '" + ichannel + "' does not exist.");
                        }
                        this->mapping.emplace_back((uint16_t) std::stoi(uchannel), input_channels.eight_bit_channels.at(ichannel));
                    } catch (const std::exception& e) {
                        throw filter_config_exception("Failed to configure output filter for universe " + std::to_string(this->universe_id) + ": input channel '" + ichannel + "' cannot be mapped to universe channel #" + uchannel + ". Reason: " + e.what());
                    }
                }
            }
        }

        virtual  bool receive_update_from_gui(const std::string& key, const std::string& _value) override {
            MARK_UNUSED(key);
            MARK_UNUSED(_value);
            return false;
        }

        virtual void get_output_channels(channel_mapping& map, const std::string& name) override {
            MARK_UNUSED(map);
            MARK_UNUSED(name);
        }

        virtual void update() override {
            // TODO update universe retrival to only occur on scene_activated
            if(auto uptr = dmxfish::io::get_universe(this->universe_id); uptr != nullptr) {
                for(auto& l : this->mapping) {
                    (*uptr)[l.universe_channel] = *l.input_channel;
		    //::spdlog::debug("Output {} to channel {} of universe {} of type {}", *l.input_channel, l.universe_channel, this->universe_id, (unsigned int) uptr->getUniverseType());
                }
            } else {
                throw std::invalid_argument("The requested universe with id " + std::to_string(this->universe_id) + " does not exist anymore");
            }
        }

        virtual void scene_activated() override {}
    };
    COMPILER_RESTORE("-Weffc++")

}
