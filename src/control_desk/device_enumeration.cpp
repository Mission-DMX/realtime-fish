#include "control_desk/device_enumeration.hpp"

#include <filesystem>

namespace dmxfish::control_desk {

    std::list<std::pair<std::string, midi_device_id>> enumerate_control_devices() {
        std::list<std::pair<std::string, midi_device_id>> l;
        for (const auto& entry : std::filesystem::directory_iterator("/dev/")) {
            if(auto p = std::string(entry.path()); p.rfind("/dev/midi", 0) == 0) {
                l.push_back(std::make_pair(p, midi_device_id::X_TOUCH));
            }
        }
        return l;
    }

}
