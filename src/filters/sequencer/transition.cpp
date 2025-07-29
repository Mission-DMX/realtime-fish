//
// Created by Leon Dietrich on 18.02.25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#include "filters/sequencer/transition.hpp"

#include <cstdint>
#include <stdexcept>

#include "utils.hpp"

namespace dmxfish::filters::sequencer {
    transition::transition() : name(), frames_8bit(), frames_16bit(), frames_float(), frames_color(), affected_channel_ids() {
        // Nothing to do here
    }

    transition::transition(const std::string& name_, const std::list<std::string>& s, const name_maps& nm) : transition() {
        using namespace utils;
        for (const auto& frame_def : s) {
            const auto params = split(frame_def, ':');
            const auto target_channel = get_from_str_list(params, 0);
            const auto transition = stotransition(get_from_str_list(params, 2));
            const auto duration = std::stod(get_from_str_list(params, 3));
            if (nm.name_to_id_8bit.contains(target_channel)) {
                const auto target_value = (uint8_t) std::stoi(get_from_str_list(params, 1));
                const auto target_channel_id = nm.name_to_id_8bit.at(target_channel);
                if (!this->frames_8bit.contains(target_channel_id)) {
                    this->frames_8bit.try_emplace(target_channel_id);
                }
                this->frames_8bit[target_channel_id].emplace_back(target_value, transition, duration);
            } else if (nm.name_to_id_16bit.contains(target_channel)) {
                const auto target_value = (uint16_t) std::stoi(get_from_str_list(params, 1));
                const auto target_channel_id = nm.name_to_id_16bit.at(target_channel);
                if (!this->frames_16bit.contains(target_channel_id)) {
                    this->frames_16bit.try_emplace(target_channel_id);
                }
                this->frames_16bit[target_channel_id].emplace_back(target_value, transition, duration);
            } else if (nm.name_to_id_float.contains(target_channel)) {
                const auto target_value = std::stod(get_from_str_list(params, 1));
                const auto target_channel_id = nm.name_to_id_float.at(target_channel);
                if (!this->frames_float.contains(target_channel_id)) {
                    this->frames_float.try_emplace(target_channel_id);
                }
                this->frames_float[target_channel_id].emplace_back(target_value, transition, duration);
            } else if (nm.name_to_id_color.contains(target_channel)) {
                const auto target_value = dmxfish::dmx::stopixel(get_from_str_list(params, 1));
                const auto target_channel_id = nm.name_to_id_color.at(target_channel);
                if (!this->frames_color.contains(target_channel_id)) {
                    this->frames_color.try_emplace(target_channel_id);
                }
                this->frames_color[target_channel_id].emplace_back(target_value, transition, duration);
            } else {
                throw std::invalid_argument("No channel with name '" + target_channel + "' exists.");
            }
        }
        this->name = name_;
    }
}