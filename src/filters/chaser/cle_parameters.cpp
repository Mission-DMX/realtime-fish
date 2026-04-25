#include "cle_parameters.hpp"

#include "utils.hpp"

namespace dmxfish::filters {

cle_number_parameter::cle_number_parameter() : ptr(nullptr), val(0) {}

cle_number_parameter::cle_number_parameter(const std::string& descr, std::map<std::string, uint16_t*> inputs) :
	ptr(inputs.contains(descr) ? inputs.at(descr) : nullptr), val(inputs.contains(descr) ? 0 : (uint16_t) std::stoi(descr)) {}

uint16_t cle_number_parameter::get() const {
	if(this->ptr != nullptr) {
		return *(this->ptr);
	}
	return this->val;
}

cle_number_parameter& cle_number_parameter::operator=(const cle_number_parameter& other) noexcept {
	this->ptr = other.ptr;
	this->val = other.val;
	return *this;
}

cle_color_parameter::cle_color_parameter() : val(nullptr), delete_required(false) {}

cle_color_parameter::cle_color_parameter(const std::string& descr, std::map<std::string, dmxfish::dmx::pixel*> inputs) :
	val(inputs.contains(descr) ? inputs.at(descr) : new dmxfish::dmx::pixel(0.0, 0.0, 0.0)), delete_required(!inputs.contains(descr)) {
    if(this->delete_required) {
	    auto comp_strings = utils::split(descr, ',');
	    this->val->setHue(std::stod(comp_strings.front()));
	    comp_strings.pop_front();
	    this->val->setSaturation(std::stod(comp_strings.front()));
	    comp_strings.pop_front();
	    this->val->setIluminance(std::stod(comp_strings.front()));
    }
}

cle_color_parameter::~cle_color_parameter() {
    if (this->delete_required && this->val != nullptr) {
	delete this->val;
    }
}

cle_color_parameter& cle_color_parameter::operator=(const cle_color_parameter& other) noexcept {
    if (this->delete_required) {
	if (this->val != nullptr) {
	    delete this->val;
	}
	if(other.get() != nullptr) {
            *this->val = *(other.get());
	} else {
	    this->val = new dmxfish::dmx::pixel(0.0, 0.0, 0.0);
	}
    } else if (other.delete_required) {
	this->delete_required = true;
	if (other.val != nullptr) {
	    *this->val = *(other.get());
	} else {
	    this->val = new dmxfish::dmx::pixel(0.0, 0.0, 0.0);
	}
    } else {
	this->val = other.val;
    }
    return *this;
}

}
