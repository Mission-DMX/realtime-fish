//
// Created by Leon Dietrich on 13.01.25.
//

#pragma once

#include <string>
#include <optional>
#include "proto_src/RealTimeControl.pb.h"

namespace dmxfish::execution::state_registry {

    /**
     * This method returns the value associated with that key.
     *
     * @param key The key of the value to fetch
     * @return The value associated if available
     */
    [[nodiscard]] std::optional<std::string> get(const std::string& key);

    /**
     * This method returns the value associated with that key inside the specified
     * scene id.
     *
     * @param scene_id The id of the scene the parameter belongs to
     * @param key The key of the parameter. This might be in the form filterid::parameter
     * @return The value (if any)
     */
    [[nodiscard]] std::optional<std::string> get(const size_t scene_id, const std::string& key);

    /**
     * Use this method to set a key to a value.
     *
     * @param key The key to update the value from
     * @param value The value to set
     */
    void set(const std::string& key, const std::string& value);

    /**
     * Use this method in order to set a value belonging to a key of a specified scene
     *
     * @param scene_id The ID of the scene
     * @param key The key to update
     * @param value The value to set
     */
    void set(const size_t scene_id, const std::string& key, const std::string& value);

    /**
     * This method updates the state registry if the provided message is not empty.
     * Otherwise it will fill it with the current state.
     *
     * @param msg The message to handle or fill
     * @return True if the message was filled and needs to be transmitted.
     */
    [[nodiscard]] bool update_states_from_message(::missiondmx::fish::ipcmessages::state_list& msg);
}
