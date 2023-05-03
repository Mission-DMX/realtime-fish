#include "control_desk/desk.hpp"
#include "control_desk/xtouch_driver.hpp"

#include "proto_src/MessageTypes.pb.h"

#include "io/iomanager.hpp"
#include "lib/logging.hpp"

namespace dmxfish::control_desk {

    bank::bank(std::function<void(std::string const&, bool)> handler) : columns{}, set_ready_state_handler{std::move(handler)} {

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
                size_of_bank_on_new_set = nbs.fader_banks[nbs.active_bank]->size();
            }
            if(cbs.active_bank < cbs.fader_banks.size()) {
                cbs.fader_banks[cbs.active_bank]->deactivate(size_of_bank_on_new_set);
            }
            current_active_bank_set = index;
            cbs = bank_sets[current_active_bank_set];
            if(!(cbs.active_bank < cbs.fader_banks.size())) {
                cbs.active_bank = 0;
            }
            if(cbs.active_bank < cbs.fader_banks.size()) {
                cbs.fader_banks[cbs.active_bank]->activate();
            }
	    // TODO display bank set id on 7seg for a short period of time
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
        cbs.fader_banks[cbs.active_bank]->deactivate(cbs.fader_banks[index]->size());
        cbs.active_bank = index;
        cbs.fader_banks[cbs.active_bank]->activate();
        for(auto d : devices) {
            d->schedule_transmission();
        }
        return true;
    }

    void desk::reset_devices() {
        if(current_active_bank_set < bank_sets.size()) {
            auto& cbs = this->bank_sets[current_active_bank_set];
            if(cbs.active_bank < cbs.fader_banks.size()) {
                cbs.fader_banks[cbs.active_bank]->deactivate(0);
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
		    for(auto i = (uint8_t) fader::FADER_CH1; i <= (uint8_t) fader::FADER_MAIN; i++) {
			    // TODO clean up code (or implement also for xtouch ext.)
			    xtouch_set_fader_position(*d, fader{i}, 0);
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

    void desk::process_incomming_command(const midi_command& c, size_t device_index) {
        if(this->iomanager == nullptr) {
            return;
        }
        auto& d = this->devices[device_index];
        switch(d->get_device_id()) {
            case midi_device_id::X_TOUCH_EXTENSION:
            case midi_device_id::X_TOUCH:
                switch (c.status) {
                    case midi_status::NOTE_ON: {
                        const button b{c.data_1};
                        if(xtouch_is_column_button(b)) {
                            // NOTE if we would ever support input devices with anything but 8 columns, this would be a bug.
                            const auto column_index = (device_index * 8) + (c.data_1 - XTOUCH_FADER_INDEX_OFFSET);
                            ::missiondmx::fish::ipcmessages::button_state_change msg;
                            if(current_active_bank_set < bank_sets.size()) {
                                auto& cbs = this->bank_sets[current_active_bank_set];
                                if(cbs.active_bank < cbs.fader_banks.size()) {
                                    auto col_ptr = cbs.fader_banks[cbs.active_bank]->get(column_index);
                                    col_ptr->process_button_press_message(b, button_change{c.data_2});
                                    msg.set_button((unsigned int) (device_index * 256 + c.data_1));
                                    msg.set_new_state(c.data_2 > 10 ? ::missiondmx::fish::ipcmessages::BS_BUTTON_PRESSED : ::missiondmx::fish::ipcmessages::BS_BUTTON_RELEASED);
                                    iomanager->push_msg_to_all_gui(msg, ::missiondmx::fish::ipcmessages::MSGT_BUTTON_STATE_CHANGE);
                                }
                            }
                        } else if(xtouch_is_fader_touch(b)) {
                            ::missiondmx::fish::ipcmessages::button_state_change msg;
                            msg.set_button((unsigned int) (device_index * 256 + c.data_1));
                            msg.set_new_state(c.data_2 > 10 ? ::missiondmx::fish::ipcmessages::BS_BUTTON_PRESSED : ::missiondmx::fish::ipcmessages::BS_BUTTON_RELEASED);
                            iomanager->push_msg_to_all_gui(msg, ::missiondmx::fish::ipcmessages::MSGT_BUTTON_STATE_CHANGE);
                        } else {
                            this->handle_bord_buttons(b, button_change{c.data_2});
                        }
                        break;
                    } case midi_status::CONTROL_CHANGE:
                        if(xtouch_is_column_fader(c.data_1)) {
                            // NOTE if we would ever support input devices with anything but 8 columns, this would be a bug.
                            const auto column_index = (device_index * 8) + (c.data_1 - XTOUCH_FADER_INDEX_OFFSET);
                            ::missiondmx::fish::ipcmessages::fader_position msg;
                            if(current_active_bank_set < bank_sets.size()) {
                                auto& cbs = this->bank_sets[current_active_bank_set];
                                if(cbs.active_bank < cbs.fader_banks.size()) {
                                    auto col_ptr = cbs.fader_banks[cbs.active_bank]->get(column_index);
                                    auto value = (c.data_2 < 127 ? c.data_2 : 128) * (65536 / 128);
                                    col_ptr->process_fader_change_message(value);
                                    msg.set_column_id(col_ptr->get_id());
                                    msg.set_position(value);
                                    iomanager->push_msg_to_all_gui(msg, ::missiondmx::fish::ipcmessages::MSGT_FADER_POSITION);
                                }
                            }
                        } else if(xtouch_is_column_encoder(c.data_1)) {
                            // NOTE if we would ever support input devices with anything but 8 columns, this would be a bug.
                            const auto column_index = (device_index * 8) + (c.data_1 - XTOUCH_ENCODER_INDEX_OFFSET);
                            if(current_active_bank_set < bank_sets.size()) {
                                auto& cbs = this->bank_sets[current_active_bank_set];
                                if(cbs.active_bank < cbs.fader_banks.size()) {
                                    ::missiondmx::fish::ipcmessages::rotary_encoder_change msg;
                                    auto col_ptr = cbs.fader_banks[cbs.active_bank]->get(column_index);
                                    const auto change = c.data_2 == 65 ? 1 : -1;
                                    col_ptr->process_encoder_change_message(change);
                                    msg.set_column_id(col_ptr->get_id());
                                    msg.set_amount(change);
                                    iomanager->push_msg_to_all_gui(msg, ::missiondmx::fish::ipcmessages::MSGT_ROTARY_ENCODER_CHANGE);
                                }
                            }
                        } else {
                            // TODO handle jogwheel
                            // TODO foot switches
                        }
                        break;
                    case midi_status::INVALID:
                    case midi_status::NOTE_OFF:
                    case midi_status::POLYPHONIC:
                    case midi_status::PROG_CHANGE:
                    case midi_status::MONOPHONIC:
                    case midi_status::PITCH_BEND:
                    case midi_status::SYS_EX:
                    default:
                        ::spdlog::error("unexpected MIDI command status: {}.", (int) c.status);
                }
                break;
            default:
                ::spdlog::error("control desk device ID {} not yet implemented for input processing.", (int) d->get_device_id());
                break;
        }
        d->schedule_transmission();
    }

    void desk::handle_bord_buttons(button b, button_change c) {
        if(b == button::BTN_EQ_COMMITRDY) {
            if(c != button_change::PRESS) {
                return;
            }
            if(!(current_active_bank_set < bank_sets.size())) {
                return;
            }
            for(auto& bs = bank_sets[current_active_bank_set]; auto& column_id : bs.columns_in_ready_state) {
                if(bs.columns_map.contains(column_id)) {
                    bs.columns_map.at(column_id)->commit_from_readymode();
                } else {
                    ::spdlog::error("Error in commiting desk ready state: Column id {} in ready set but not in index map.", column_id);
                }
            }
        } else {
            ::spdlog::error("Handling button {} not yet implemented in input desk handler.", (uint8_t) b);
        }
        // TODO implement
    }

    void desk::update() {
        for (size_t i = 0; i < this->devices.size(); i++) {
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
                        bs.fader_banks[bs.active_bank]->deactivate(0);
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

    [[nodiscard]] inline bank_mode deduce_bank_mode(const ::missiondmx::fish::ipcmessages::fader_column& bank_definition) {
        if(bank_definition.has_plain_color())
            return bank_mode::HSI_COLOR_MODE;
        if(bank_definition.has_color_with_uv())
            return bank_mode::HSI_WITH_UV_MODE;
        if(bank_definition.has_raw_data())
            return bank_mode::DIRECT_INPUT_MODE;
        return bank_mode::HSI_WITH_AMBER_AND_UV_MODE; // Should not happen with current buffer version
    }

    void desk::add_bank_set_from_protobuf_msg(const ::missiondmx::fish::ipcmessages::add_fader_bank_set& definition) {
        bank_sets.emplace_back();
        const auto new_bank_index = bank_sets.size() - 1;
        bankset_to_index_map[definition.bank_id()] = new_bank_index;
        auto& bs = bank_sets[new_bank_index];
        bs.fader_banks.reserve(definition.banks_size());
        for (auto& bank_definition : definition.banks()) {
            using namespace std::placeholders;
            bs.fader_banks.push_back(std::make_shared<bank>(std::bind(&desk::handle_ready_state_update_from_bank, this, _1, _2)));
            bs.fader_banks[bs.fader_banks.size() - 1]->reserve(bank_definition.cols_size());
            size_t device_index = 0;
            unsigned int col_index_on_device = 0;
            for(auto& col_definition : bank_definition.cols()) {
                if(device_index == devices.size()) {
                    // We have too many columns to represent with our devices.
                    break;
                }
                if(col_index_on_device == devices[device_index]->get_number_of_supported_columns()) {
                    device_index++;
                    col_index_on_device = 0;
                }
                auto& selected_device = devices[device_index];
                const auto& id = col_definition.column_id();
                auto col_ptr = bs.fader_banks[bs.fader_banks.size() - 1]->emplace_back(selected_device, deduce_bank_mode(col_definition), id, (uint8_t) col_index_on_device);
                bs.columns_map[id] = col_ptr;
                col_index_on_device++;
            }
        }
        // ready set does not need to be populated as they are not in ready state by default
        bs.active_bank = definition.default_active_fader_bank();
	if(bank_sets.size() == 1) {
		// Enable first bank if it's the only one
		this->set_active_bank_set(0);
	}
    }

    void desk::update_fader_position_from_protobuf(const ::missiondmx::fish::ipcmessages::fader_position& msg) {
        if(!(current_active_bank_set < bank_sets.size())) {
            return;
        }
        auto& bs = bank_sets[current_active_bank_set];
        const auto& cid = msg.column_id();
        if(!bs.columns_map.contains(cid)) {
            return;
        }
        bs.columns_map.at(cid)->process_fader_change_message(msg.position());
    }

    void desk::update_encoder_state_from_protobuf(const ::missiondmx::fish::ipcmessages::rotary_encoder_change& msg) {
        if(!(current_active_bank_set < bank_sets.size())) {
            return;
        }
        auto& bs = bank_sets[current_active_bank_set];
        const auto& cid = msg.column_id();
        if(!bs.columns_map.contains(cid)) {
            return;
        }
        bs.columns_map.at(cid)->process_encoder_change_message(msg.amount());
    }

    void desk::update_button_leds_from_protobuf(const missiondmx::fish::ipcmessages::button_state_change& msg) {
        if(msg.button() < 0 || msg.button() > xtouch_biggest_led_index) {
            return;
        }
        button b = button{(uint8_t) msg.button()};
        button_led_state s;
        switch(msg.new_state()) {
            case missiondmx::fish::ipcmessages::BS_ACTIVE:
                s = button_led_state::on;
                break;
            case missiondmx::fish::ipcmessages::BS_SET_LED_BLINKING:
                s = button_led_state::flash;
                break;
            case missiondmx::fish::ipcmessages::BS_SET_LED_NOT_ACTIVE:
            case missiondmx::fish::ipcmessages::BS_BUTTON_PRESSED:
            case missiondmx::fish::ipcmessages::BS_BUTTON_RELEASED:
            case missiondmx::fish::ipcmessages::ButtonState_INT_MIN_SENTINEL_DO_NOT_USE_:
            case missiondmx::fish::ipcmessages::ButtonState_INT_MAX_SENTINEL_DO_NOT_USE_:
            default:
                s = button_led_state::off;
                break;
        }
        // TODO specify which device by calculating modulo of msg instead of rejecting it.
        for(auto& d : devices) {
            xtouch_set_button_led(*d, b, s);
            d->schedule_transmission();
        }
    }

    void desk::handle_ready_state_update_from_bank(const std::string& column_id, bool new_state) {
        if(!(current_active_bank_set < bank_sets.size())) {
            return;
        }
        auto& rset = bank_sets[current_active_bank_set].columns_in_ready_state;
        if(new_state) {
            rset.insert(column_id);
        } else {
            rset.erase(column_id);
        }
        for(auto btn_state = rset.empty() ? button_led_state::off : button_led_state::flash ; auto& d_ptr : devices) {
            if(d_ptr->get_device_id() == midi_device_id::X_TOUCH) {
                xtouch_set_button_led(*d_ptr, button::BTN_EQ_COMMITRDY, btn_state);
                d_ptr->schedule_transmission();
            }
        }
    }
}
