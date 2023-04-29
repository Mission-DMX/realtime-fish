#include "control_desk/desk.hpp"
#include "control_desk/xtouch_driver.hpp"

namespace dmxfish::control_desk {

    desk::desk(std::list<std::pair<std::string, midi_device_id>> input_devices) : devices{}, fader_banks{} {
        devices.reserve(input_devices.size());
        for(const auto& entry : input_devices) {
            devices.push_back(std::make_shared<device_handle>(entry.first, entry.second));
        }
        this->reset_devices();
    }

    desk::~desk() {
        this->reset_devices();
    }

    void desk::reset_devices() {
        // TODO deactivate all bank columns (transmission scheduling is called with final device reset)
        for(auto d : this->devices) {
            switch (d->get_device_id()) {
                case midi_device_id::X_TOUCH:
                    for(auto b : xtouch_buttons()) {
                        xtouch_set_button_led(*d, b, button_led_state::off);
                    }
                    // TODO reset 7seg display
                    break;
                case midi_device_id::X_TOUCH_EXTENSION:
                    for(auto b : xtouch_extender_buttons()) {
                        xtouch_set_button_led(*d, b, button_led_state::off);
                    }
                    break;
                default:
                    break;
            }
            d->schedule_transmission();
        }
    }
}
