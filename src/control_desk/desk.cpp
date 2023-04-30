#include "control_desk/desk.hpp"
#include "control_desk/xtouch_driver.hpp"

namespace dmxfish::control_desk {

    bank::bank() : columns{} {

    }

    void bank::activate() {
        for(auto& d : columns) {
            d->set_active(true);
        }

    }

    void bank::deactivate(size_t columns_in_next_row) {
        for(auto& d : columns) {
            d->set_active(false);
        }
        for(auto i = columns_in_next_row; i < columns.size(); i++) {
            columns[i]->reset_column();
        }
    }

    desk::desk(std::list<std::pair<std::string, midi_device_id>> input_devices) : devices{}, bank_sets{}, bankset_to_index_map{} {
        devices.reserve(input_devices.size());
        for(const auto& entry : input_devices) {
            devices.push_back(std::make_shared<device_handle>(entry.first, entry.second));
        }
        this->reset_devices();
    }

    desk::~desk() {
        this->reset_devices();
    }

    bool desk::set_active_bank_set(size_t index) {
        if(index < bank_sets.size() && index != current_active_bank_set) {
            auto& cbs = bank_sets[current_active_bank_set];
            size_t size_of_bank_on_new_set = 0;
            if(auto& nbs = bank_sets[index]; nbs.active_bank < nbs.fader_banks.size()) {
                size_of_bank_on_new_set = nbs.fader_banks[nbs.active_bank].size();
            }
            if(cbs.active_bank < cbs.fader_banks.size()) {
                cbs.fader_banks[cbs.active_bank].deactivate(size_of_bank_on_new_set);
            }
            current_active_bank_set = index;
            cbs = bank_sets[current_active_bank_set];
            if(!(cbs.active_bank < cbs.fader_banks.size())) {
                cbs.active_bank = 0;
            }
            if(cbs.active_bank < cbs.fader_banks.size()) {
                cbs.fader_banks[cbs.active_bank].activate();
            }
            for(auto d : devices) {
                d->schedule_transmission();
            }
            return true;
        } else {
            return false;
        }
    }

    bool desk::set_active_fader_bank_on_current_set(size_t index) {
        if(current_active_bank_set < bank_sets.size()) {
            return false;
        }
        auto& cbs = bank_sets[current_active_bank_set];
        if(index < cbs.fader_banks.size()) {
            return false;
        }
        cbs.fader_banks[cbs.active_bank].deactivate(cbs.fader_banks[index].size());
        cbs.active_bank = index;
        cbs.fader_banks[cbs.active_bank].activate();
        for(auto d : devices) {
            d->schedule_transmission();
        }
        return true;
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
