#pragma once

#include <list>
#include <string>
#include <utility>

#include "control_desk/command.hpp"

namespace dmxfish::control_desk {
    std::list<std::pair<std::string, midi_device_id>> enumerate_control_devices();
}
