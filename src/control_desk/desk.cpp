#include "control_desk/desk.hpp"
#include "control_desk/xtouch_driver.hpp"

#include "lib/logging.hpp"

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
            auto d = std::make_shared<device_handle>(entry.first, entry.second);
            devices.push_back(d);
            switch(d->get_device_id()) {
                case midi_device_id::X_TOUCH:
                    this->max_number_of_colums += 8;
                    break;
                case midi_device_id::X_TOUCH_EXTENSION:
                    this->max_number_of_colums += 8;
                    break;
                default:
                    break;
            }
        }
        this->reset_devices();
        if(devices.size() == 0) {
            ::spdlog::warn("No input devices where added to the control desk.");
        } else {
            ::spdlog::debug("Added {} devices to control desk.", devices.size());
        }
    }

    desk::~desk() {
        this->reset_devices();
        ::spdlog::debug("Stopping control desk handler. Reseted connected input devices.");
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
        if(current_active_bank_set < bank_sets.size()) {
            auto& cbs = this->bank_sets[current_active_bank_set];
            if(cbs.active_bank < cbs.fader_banks.size()) {
                cbs.fader_banks[cbs.active_bank].deactivate(0);
            }
        }
        for(auto d : this->devices) {
            switch (d->get_device_id()) {
                case midi_device_id::X_TOUCH:
                    for(auto b : xtouch_buttons()) {
                        xtouch_set_button_led(*d, b, button_led_state::off);
                    }
                    {
                        const std::array<char, 12> empty_7seg_data = {'-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-'};
                        xtouch_set_seg_display(*d, empty_7seg_data);
                    }
                    break;
                case midi_device_id::X_TOUCH_EXTENSION:
                    for(auto b : xtouch_extender_buttons()) {
                        xtouch_set_button_led(*d, b, button_led_state::off);
                    }
                    break;
                default:
                    ::spdlog::error("control desk device ID {} not yet implemented for reset.", (int) d->get_device_id());
                    break;
            }
            d->schedule_transmission();
        }
    }

    void desk::process_incomming_command(midi_command& c, size_t device_index) {
        auto& d = this->devices[device_index];
        switch(d->get_device_id()) {
            case midi_device_id::X_TOUCH_EXTENSION:
            case midi_device_id::X_TOUCH:
                // TODO route column commands
                switch (c.status) {
                    case midi_status::NOTE_ON:
                        if(xtouch_is_column_button(button{c.data_1})) {
                            // TODO route button press to corresponding column
                        } else if(xtouch_is_fader_touch(button{c.data_1})) {
                            // TODO notify GUI about fader touch
                        } else {
                            // TODO handle bord buttons
                        }
                        break;
                    case midi_status::CONTROL_CHANGE:
                        // TODO handle jogwheel, faders and encoders
                    default:
                        ::spdlog::error("unexpected MIDI command status: {}.", (int) c.status);
                }
                break;
            default:
                ::spdlog::error("control desk device ID {} not yet implemented for input processing.", (int) d->get_device_id());
                break;
        }
    }

    void desk::update() {
        for (int i = 0; i < this->devices.size(); i++) {
            auto& d = this->devices[i];
            auto c = d->get_next_command_from_desk();
            while(c) {
                process_incomming_command(c.value(), i);
                c = d->get_next_command_from_desk();
            }
            d->schedule_transmission();
        }
    }

    void desk::remove_bank_set(size_t i) {
        if(const auto s = bank_sets.size(); i + 1 > s) {
            ::spdlog::error("Tried to remove fader bank set {}. The container contains only {} elements though.", i, s);
            return;
        } else if(i + 1 == s) {
            if (i == current_active_bank_set) {
                if (s > 1) {
                    // if we're erasing the active one and we're not the last one, call set_active_bank_set with i - 1
                    set_active_bank_set(i - 1);
                } else {
                    // otherwise call deactivate on the current bank set
                    auto& bs = bank_sets[i];
                    if(bs.active_bank < bs.fader_banks.size()) {
                        bs.fader_banks[bs.active_bank].deactivate(0);
                    }
                }
            }
        } else {
            // swap set with last one if it isn't the last one (by copying the last one to the one being removed)
            this->bank_sets[i] = this->bank_sets[s - 1];
            // update the id of the last element in index map
            for(const auto& [k, v] : bankset_to_index_map) {
                if(v == s - 1) {
                    bankset_to_index_map[k] = i;
                    break;
                }
            }
            // if the swapped set was the active one, call set_active_bank_set with the new index
            if (current_active_bank_set == s - 1) {
                set_active_bank_set(i);
            }
        }
        // call erase on the last element
        bank_sets.pop_back();
        // We do not call d->schedule_transmission(); here as there might be further removal operations and we'd be updating the desk 20ms later anyway.
    }
}
