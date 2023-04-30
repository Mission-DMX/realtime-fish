#include "control_desk/bank_column.hpp"

namespace dmxfish::control_desk {

	bank_column::bank_column(std::weak_ptr<device_handle> device_connection, bank_mode mode, std::string _id) :
		connection(device_connection), id(_id), display_text_up{}, display_text_down{}, color{}, readymode_color{}, raw_configuration{}, readymode_raw_configuration{}, current_bank_mode(mode) {}

	void bank_column::set_active(bool new_value) {
		this->active_on_device = new_value;
		if(new_value) {
			// TODO implement activation
		} else {
			// TODO reset faders to 0
            // TODO set side leds to 0
			// TODO disable rotary encoder leds
            // TODO clear lcd displays
		}
	}

	void bank_column::reset_column() {
		// TODO implement
	}

}
