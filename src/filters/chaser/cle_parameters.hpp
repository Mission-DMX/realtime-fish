#pragma once

#include <map>
#include <string>

#include "dmx/pixel.hpp"

namespace dmxfish::filters {
    class cle_number_parameter {
        private:
            uint16_t* ptr;
            uint16_t val = 0;
        public:
	    cle_number_parameter();
	    cle_number_parameter(const cle_number_parameter& other) = default;
	    cle_number_parameter(cle_number_parameter&& other) = default;
            cle_number_parameter(const std::string& descr, std::map<std::string, uint16_t*> inputs);
	    cle_number_parameter& operator=(const cle_number_parameter& other) noexcept;
            uint16_t get() const;
    };

    class cle_color_parameter {
        private:
            dmxfish::dmx::pixel* val;
            bool delete_required;
        public:
	    cle_color_parameter();
	    cle_color_parameter(const cle_color_parameter& other) = default;
	    cle_color_parameter(cle_color_parameter&& other) = default;
            cle_color_parameter(const std::string& descr, std::map<std::string, dmxfish::dmx::pixel*> inputs);
	    cle_color_parameter& operator=(const cle_color_parameter& other) noexcept;
            ~cle_color_parameter();
            inline dmxfish::dmx::pixel* get() {
		    return this->val;
	    }

	    inline const dmxfish::dmx::pixel* get() const {
		    return this->val;
	    }
    };
}
