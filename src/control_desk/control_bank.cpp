#include "control_desk/control_bank.hpp"

namespace dmxfish::control_desk {

	bank_column::bank_column(std::shared_ptr<device_handle> device_connection, bank_mode mode, std::string id) : connection(device_connection), id(id), display_text_up{}, display_text_down{}, current_bank_mode(mode) {}

	void bank_column::set_active(bool new_value) {
		// TODO implement
	}

}
