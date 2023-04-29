#pragma once

#include <list>
#include <memory>
#include <utility>
#include <vector>

#include "control_desk/bank_column.hpp"
#include "control_desk/command.hpp"
#include "control_desk/device_handle.hpp"

namespace dmxfish::control_desk {

    class desk {
    private:
        std::vector<std::shared_ptr<device_handle>> devices;
        std::vector<bank_column> fader_banks;
    public:
        desk(std::list<std::pair<std::string, midi_device_id>> input_devices);
        ~desk();
    private:
        void reset_devices();
    };

}
