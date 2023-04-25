#pragma once

#include <memory>

#include "control_desk/device_handle.hpp"

namespace dmxfish::control_desk {

    // TODO implement image and text with font support

    class display_content {
    private:
        uint64_t address_on_desk = 0;
    public:
        display_content(std::shared_ptr<device_handle> connection);
    };

}
