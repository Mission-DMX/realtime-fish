//
// Created by Leon Dietrich on 13.01.25.
//
#include "state_registry.hpp"

#include <unordered_map>

namespace dmxfish::execution::state_registry {
    std::unordered_map<std::string, std::string> scene_specific_map, unspecific_map;

    void set(const size_t scene_id, const std::string& key, const std::string& value) {
        scene_specific_map[std::to_string(scene_id) + "::" + key] = value;
    }

    void set(const std::string& key, const std::string& value) {
        unspecific_map[key] = value;
    }

    [[nodiscard]] std::optional<std::string> get(const std::string& key) {
        if (!unspecific_map.contains(key)) {
            return std::nullopt;
        }
        return unspecific_map.at(key);
    }

    [[nodiscard]] std::optional<std::string> get(const size_t scene_id, const std::string& key) {
        const auto akey = std::to_string(scene_id) + "::" + key;
        if (!unspecific_map.contains(akey)) {
            return std::nullopt;
        }
        return unspecific_map.at(akey);
    }
}