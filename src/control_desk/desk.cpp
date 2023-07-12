#include "control_desk/desk.hpp"
#include "control_desk/xtouch_driver.hpp"

#include "proto_src/MessageTypes.pb.h"

#include "io/iomanager.hpp"
#include "lib/logging.hpp"
#include "main.hpp"

#include <iostream>

namespace dmxfish::control_desk {

    bank::bank(std::function<void(std::string const&, bool)> rdy_handler, std::function<void(std::string const&, bool)> select_handler) :
        columns{}, set_ready_state_handler{std::move(rdy_handler)}, select_state_handler{std::move(select_handler)} {

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

    void bank::update() {
        if (current_col_update_index < columns.size()) {
            columns[current_col_update_index]->update();
            current_col_update_index++;
        } else {
            current_col_update_index = 0;
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
	for(auto& d : devices) {
            if(d->get_device_id() == midi_device_id::X_TOUCH) {
                std::array<char, 14> initial_lcd_text = {'M', 'i', 's', 's', 'i', 'o', 'n', 'V', 'e', 'r', 's', 'i', 'o', 'n'};
                xtouch_set_lcd_display(*d, 0, lcd_color::white_up_inverted, initial_lcd_text);
                initial_lcd_text = {' ', 'D', 'M', 'X', ' ', ' ', ' ', ' ', '1', '.', '0', '.', '0', ' '};
                xtouch_set_lcd_display(*d, 1, lcd_color::white_up_inverted, initial_lcd_text);
	        d->schedule_transmission();
	    }
	}
        if(devices.size() == 0) {
            ::spdlog::warn("No input devices where added to the control desk.");
        } else {
            ::spdlog::debug("Added {} devices to control desk.", devices.size());
        }
    }

    desk::~desk() {
        this->reset_devices();
	get_iomanager_instance()->handle_queued_io();
        ::spdlog::debug("Stopping control desk handler. Reseted connected input devices.");
    }

    bool desk::set_active_bank_set(size_t index) {
        if(index < bank_sets.size() && (index != current_active_bank_set || bank_set_modification_happened)) {
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
            update_fader_bank_leds();
            update_message_required = true;
	    bank_set_modification_happened = false;
            return true;
        } else {
            return false;
        }
    }

    void desk::update_fader_bank_leds() {
	if(!(current_active_bank_set < bank_sets.size())) {
            return;
        }
	const auto& cbs = bank_sets[current_active_bank_set];
	const auto index = cbs.active_bank;
	for(auto d : devices) {
	    xtouch_set_button_led(*d, button::BTN_FADERBANKPREV_FADERBANKPREV, index == 0 ? button_led_state::off : button_led_state::on);
            xtouch_set_button_led(*d, button::BTN_FADERBANKNEXT_FADERBANKNEXT, index + 1 == cbs.fader_banks.size() ? button_led_state::off : button_led_state::on);
            d->schedule_transmission();
        }
    }

    bool desk::set_active_fader_bank_on_current_set(size_t index) {
        if(!(current_active_bank_set < bank_sets.size())) {
            return false;
        }
        auto& cbs = bank_sets[current_active_bank_set];
        if(index >= cbs.fader_banks.size()) {
	    ::spdlog::warn("Not changing active bank on set: index out of range.");
            return false;
        }
        if(index == cbs.active_bank && !bank_set_modification_happened) {
	    //::spdlog::warn("Not changing active bank on set: already active.");
            return false;
        }
        cbs.fader_banks[cbs.active_bank]->deactivate(cbs.fader_banks[index]->size());
        cbs.active_bank = index;
        cbs.fader_banks[cbs.active_bank]->activate();
        update_fader_bank_leds();
        update_message_required = true;
        return true;
    }

    size_t desk::get_active_fader_bank_on_current_set() {
	if(!(current_active_bank_set < bank_sets.size())) {
            return 0;
        }
        auto& cbs = bank_sets[current_active_bank_set];
	return cbs.active_bank;
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
			const std::array<char, 14> empty_lcd_data{' '};
			for(uint8_t i = 0; i < XTOUCH_COLUMN_COUNT; i++) {
                            xtouch_set_lcd_display(*d, i, lcd_color{i}, empty_lcd_data);
			}
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
        auto iom = get_iomanager_instance();
        if(iom == nullptr) {
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
                            const auto column_index = (device_index * 8) + (c.data_1 % XTOUCH_COLUMN_COUNT);
                            ::missiondmx::fish::ipcmessages::button_state_change msg;
                            if(current_active_bank_set < bank_sets.size()) {
                                auto& cbs = this->bank_sets[current_active_bank_set];
                                if(cbs.active_bank < cbs.fader_banks.size()) {
                                    auto col_ptr = cbs.fader_banks[cbs.active_bank]->get(column_index);
				    if(!col_ptr) {
					::spdlog::warn("Pressed button for column {} but it is not utilized.", column_index);
					return;
				    }
                                    col_ptr->process_button_press_message(b, button_change{c.data_2});
                                    msg.set_button(missiondmx::fish::ipcmessages::ButtonCode{(unsigned int) (device_index * 256 + c.data_1)});
                                    msg.set_new_state(c.data_2 > 10 ? ::missiondmx::fish::ipcmessages::BS_BUTTON_PRESSED : ::missiondmx::fish::ipcmessages::BS_BUTTON_RELEASED);
                                    iom->push_msg_to_all_gui(msg, ::missiondmx::fish::ipcmessages::MSGT_BUTTON_STATE_CHANGE);
                                }
                            }
                        } else if(xtouch_is_fader_touch(b)) {
                            ::missiondmx::fish::ipcmessages::button_state_change msg;
                            msg.set_button(missiondmx::fish::ipcmessages::ButtonCode{(unsigned int) (device_index * 256 + c.data_1)});
                            msg.set_new_state(c.data_2 > 10 ? ::missiondmx::fish::ipcmessages::BS_BUTTON_PRESSED : ::missiondmx::fish::ipcmessages::BS_BUTTON_RELEASED);
                            iom->push_msg_to_all_gui(msg, ::missiondmx::fish::ipcmessages::MSGT_BUTTON_STATE_CHANGE);
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
				    if(!col_ptr) {
					::spdlog::warn("Fader {} moved but column is not utilized.", column_index);
					return;
				    }
                                    auto value = (c.data_2 < 127 ? c.data_2 : 128) * (65536 / 128);
                                    col_ptr->process_fader_change_message(value);
                                    msg.set_column_id(col_ptr->get_id());
                                    msg.set_position(value);
                                    iom->push_msg_to_all_gui(msg, ::missiondmx::fish::ipcmessages::MSGT_FADER_POSITION);
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
				    if (!col_ptr) {
					::spdlog::warn("Encoder {} used but column is not utilized.", column_index);
					return;
				    }
                                    const auto change = c.data_2 == 65 ? 1 : -1;
                                    col_ptr->process_encoder_change_message(change);
                                    msg.set_column_id(col_ptr->get_id());
                                    msg.set_amount(change);
                                    iom->push_msg_to_all_gui(msg, ::missiondmx::fish::ipcmessages::MSGT_ROTARY_ENCODER_CHANGE);
                                }
                            } else {
                                ::spdlog::warn("Received Column encoder change from {} to {} but no bank set is active.", c.data_1 - XTOUCH_ENCODER_INDEX_OFFSET, c.data_2);
                            }
                        } else {
                            if(c.data_1 == CMD_JOGWHEEL) {
                                jogwheel_change += c.data_2 > 60 ? 1 : -1;
                                update_message_required = true;
                            } else if(c.data_1 == (uint8_t) fader::FADER_MAIN) {
				this->global_illumination = (uint16_t) c.data_2;
			    }
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

    void desk::commit_readymode() {
        if(!(current_active_bank_set < bank_sets.size())) {
            return;
        }
        auto& bs = bank_sets[current_active_bank_set];
        for(auto& column_id : bs.columns_in_ready_state) {
            if(bs.columns_map.contains(column_id)) {
                bs.columns_map.at(column_id)->commit_from_readymode();
            } else {
                ::spdlog::error("Error in commiting desk ready state: Column id {} in ready set but not in index map.", column_id);
            }
        }
	bs.columns_in_ready_state.clear();
	for (auto d_ptr : devices) {
	    if(d_ptr->get_device_id() == midi_device_id::X_TOUCH)
                xtouch_set_button_led(*d_ptr, button::BTN_EQ_COMMITRDY, button_led_state::off);
	}
    }

    void desk::handle_bord_buttons(button b, button_change c) {
        if(b == button::BTN_EQ_COMMITRDY) {
            if(c != button_change::PRESS) {
                return;
            }
            commit_readymode();
        } else if(b == button::BTN_FADERBANKPREV_FADERBANKPREV) {
	    if(c != button_change::PRESS) {
		return;
	    }
            const auto current_bank = this->get_active_fader_bank_on_current_set();
            this->set_active_fader_bank_on_current_set(current_bank - 1); // Bounds check is performed by setter.
        } else if(b == button::BTN_FADERBANKNEXT_FADERBANKNEXT) {
	    if (c != button_change::PRESS) {
		return;
	    }
            const auto current_bank = this->get_active_fader_bank_on_current_set();
            this->set_active_fader_bank_on_current_set(current_bank + 1);
        } else if(b == button::BTN_SEND_OOPS) {
            // TODO implement
        } else if(b == button::BTN_FLIP_MAINDARK) {
            if (c != button_change::PRESS) {
		return;
            }
            this->global_dark = !this->global_dark;
	    for(auto d_ptr : devices) {
                xtouch_set_button_led(*d_ptr, button::BTN_FLIP_MAINDARK, global_dark ? button_led_state::flash : button_led_state::off);
            }
        } else if(b == button::BTN_SEND_OOPS) {
            get_iomanager_instance()->rollback();
	} else {
            ::missiondmx::fish::ipcmessages::button_state_change msg;
            msg.set_button(missiondmx::fish::ipcmessages::ButtonCode{(unsigned int) (b)});
            msg.set_new_state(c == button_change::PRESS ? ::missiondmx::fish::ipcmessages::BS_BUTTON_PRESSED : ::missiondmx::fish::ipcmessages::BS_BUTTON_RELEASED);
            get_iomanager_instance()->push_msg_to_all_gui(msg, ::missiondmx::fish::ipcmessages::MSGT_BUTTON_STATE_CHANGE);
        }
    }

    void desk::update() {
	// TODO Call column update
        const auto active_bs_valid = this->current_active_bank_set < bank_sets.size();
	const auto& active_bank_set = this->bank_sets[this->current_active_bank_set];
        if (active_bs_valid && active_bank_set.active_bank < active_bank_set.fader_banks.size())
	    active_bank_set.fader_banks[active_bank_set.active_bank]->update();
        for (size_t i = 0; i < this->devices.size(); i++) {
            auto& d = this->devices[i];
            auto c = d->get_next_command_from_desk();
            while(c) {
                process_incomming_command(c.value(), i);
                c = d->get_next_command_from_desk();
            }
            d->schedule_transmission();
        }
        if(update_message_required) {
            update_message_required = false;
            ::missiondmx::fish::ipcmessages::desk_update msg;
            msg.set_selected_column_id(selected_column_id);
            msg.set_find_active_on_column_id(find_enabled_on_column_id);
            msg.set_jogwheel_change_since_last_update(jogwheel_change);
            jogwheel_change = 0;
            if(active_bs_valid) {
                msg.set_selected_bank(bank_sets[current_active_bank_set].active_bank);
                msg.set_selected_bank_set(bank_sets[current_active_bank_set].id);
            } else {
                msg.set_selected_bank(0);
                msg.set_selected_bank_set("");
            }
            msg.set_seven_seg_display_data("");
            get_iomanager_instance()->push_msg_to_all_gui(msg, ::missiondmx::fish::ipcmessages::MSGT_DESK_UPDATE);
        }
    }

    void desk::process_desk_update_message(const ::missiondmx::fish::ipcmessages::desk_update& msg) {
        set_seven_seg_display_data(msg.seven_seg_display_data());
        set_active_bank_set(msg.selected_bank_set());
        set_active_fader_bank_on_current_set(msg.selected_bank());
        if(msg.selected_column_id() != this->selected_column_id) {
            handle_select_state_update_from_bank(msg.selected_column_id(), true);
        }
        if(msg.find_active_on_column_id().length() > 0) {
            // TODO implement
        }
    }

    void desk::set_seven_seg_display_data(const std::string& data) {
        for(auto& d : devices) {
            if(d->get_device_id() == midi_device_id::X_TOUCH) {
                std::array<char, 12> seg_data{' '};
                unsigned int i = 0;
                for (const auto& c : data) {
                    seg_data[i] = c;
                    i++;
                    if (i >= seg_data.size()) {
                        break;
                    }
                }
                xtouch_set_seg_display(*d, seg_data);
                d->schedule_transmission();
            }
        }
    }

    void desk::remove_bank_set(size_t i) {
	bank_set_modification_happened = true;
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
	bank_set_modification_happened = true;
        bank_sets.emplace_back(definition.bank_id());
        const auto new_bank_index = bank_sets.size() - 1;
        bankset_to_index_map[definition.bank_id()] = new_bank_index;
        auto& bs = bank_sets[new_bank_index];
        bs.fader_banks.reserve(definition.banks_size());
        for (auto& bank_definition : definition.banks()) {
            using namespace std::placeholders;
            bs.fader_banks.push_back(std::make_shared<bank>(
                std::bind(&desk::handle_ready_state_update_from_bank, this, _1, _2),
                std::bind(&desk::handle_select_state_update_from_bank, this, _1, _2)
            ));
            bs.fader_banks[bs.fader_banks.size() - 1]->reserve(bank_definition.cols_size());
            size_t device_index = 0;
            unsigned int col_index_on_device = 0;
            for(auto& col_definition : bank_definition.cols()) {
                if(device_index == devices.size()) {
                    // We have too many columns to represent with our devices.
		    ::spdlog::error("Skipping further columns in bank {}.", bs.fader_banks.size() - 1);
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
                update_column_from_message(col_definition, new_bank_index);
            }
        }
        // ready set does not need to be populated as they are not in ready state by default
        bs.active_bank = definition.default_active_fader_bank();
        if(bank_sets.size() == 1) {
            // Enable first bank if it's the only one
            this->set_active_bank_set(0);
        } else {
	    update_fader_bank_leds();
	}
    }

    void desk::update_column_from_message(const ::missiondmx::fish::ipcmessages::fader_column& msg, size_t bank_set_index) {
	auto active_bs_index = bank_set_index == size_t_max ? current_active_bank_set : bank_set_index;
        if(!(active_bs_index < bank_sets.size())) {
            return;
        }
        auto& cbs = bank_sets[active_bs_index];
        const auto& column_id = msg.column_id();
        if(!cbs.columns_map.contains(column_id)) {
            return;
        }
        auto col_ptr = cbs.columns_map.at(column_id);
        col_ptr->set_display_color(lcd_color{(uint8_t) msg.display_color() | (msg.top_lcd_row_inverted() ? 0b00010000 : 0) | (msg.bottom_lcd_row_inverted() ? 0b00100000 : 0)});
        col_ptr->set_display_text(msg.upper_display_text(), true);
        col_ptr->set_display_text(msg.lower_display_text(), false);
        if(msg.has_plain_color()) {
            const auto& c = msg.plain_color();
            col_ptr->set_color(dmxfish::dmx::pixel{c.hue(), c.saturation(), c.intensity()});
        } else if(msg.has_color_with_uv()) {
            const auto& c = msg.color_with_uv().base();
            col_ptr->set_uv_value(msg.color_with_uv().uv());
            col_ptr->set_color(dmxfish::dmx::pixel{c.hue(), c.saturation(), c.intensity()});
        } else {
            raw_column_configuration rc = col_ptr->get_raw_configuration();
            const auto& raw_conf_msg = msg.raw_data();
            rc.fader_position = raw_conf_msg.fader();
            rc.rotary_position = raw_conf_msg.rotary_position();
            rc.meter_leds = raw_conf_msg.meter_leds();
            // TODO handle buttons
            col_ptr->set_raw_configuration(rc);
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

    void desk::handle_select_state_update_from_bank(const std::string& column_id, bool new_state) {
        if(!(current_active_bank_set < bank_sets.size())) {
            return;
        }
        const auto& cbs = bank_sets[current_active_bank_set];
        if(cbs.columns_map.contains(selected_column_id)) {
            const auto& old_column = cbs.columns_map.at(selected_column_id);
            old_column->set_select_button_active(false);
        }
        if(cbs.columns_map.contains(column_id) && new_state) {
            selected_column_id = column_id;
        } else {
            selected_column_id = "";
        }
        if(new_state) {
            if(selected_column_id.length() > 0) {
                const auto& old_column = cbs.columns_map.at(selected_column_id);
                old_column->set_select_button_active(true);
            }
        }
        update_message_required = true;
    }

    std::shared_ptr<bank_column> desk::find_column(const std::string& set_id, const std::string& column_id) {
        if(!bankset_to_index_map.contains(set_id)) {
            return nullptr;
        }
        if(auto set_index = bankset_to_index_map.at(set_id); set_index < bank_sets.size()) {
            auto& set = bank_sets[set_index];
            if(set.columns_map.contains(column_id)) {
                return set.columns_map.at(column_id);
            } else {
                return nullptr;
            }
        } else {
            return nullptr;
        }
    }
}
