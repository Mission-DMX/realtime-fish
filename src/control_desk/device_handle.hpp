#pragma once

#include "control_desk/command.hpp"

namespace dmxfish::control_desk {
    class device_handle {
    public:
        void send_command(const command& c);
    };
}
