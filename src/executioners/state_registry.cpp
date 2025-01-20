//
// Created by Leon Dietrich on 13.01.25.
//
#include "state_registry.hpp"

#include <cstdlib>
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

    [[nodiscard]] bool update_states_from_message(::missiondmx::fish::ipcmessages::state_list& msg) {
	bool was_empty = true;
	for (const auto& [k, v] : msg.unspecific_states()) {
	    was_empty = false;
	    unspecific_map[k] = v;
	}
	for (const auto& kvs : msg.specific_states()) {
	    was_empty = false;
	    set(kvs.scene_id(), kvs.k(), kvs.v());
	}
	if(was_empty) {
	    for(const auto& [k, v] : unspecific_map) {
            (*msg.mutable_unspecific_states())[k] = v;
	    }
	    for(const auto& [sk, v]: scene_specific_map) {
            auto split_pos = sk.find("::");
            size_t scene_id = 0;
            if (split_pos == std::string::npos) {
                split_pos = 0;
                scene_id = 0;
            } else {
                // std::atol will return 0 if it fails which is exactly what we want
                scene_id = std::atol(sk.substr(0, split_pos).c_str());
                split_pos += 2;
            }
            auto* kvs = msg.add_specific_states();
            kvs->set_scene_id(scene_id);
            kvs->set_k(sk.substr(split_pos));
            kvs->set_v(v);
	    }
	}
	return !was_empty;
    }
}
