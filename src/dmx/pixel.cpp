#include "dmx/pixel.hpp"

#include <sstream>

namespace dmxfish::dmx {

    [[nodiscard]] std::string pixel::str() {
		std::stringstream ss;
        ss << "{\"hue\": " << this->hue << ", \"saturation\": " << this->saturation << ", \"iluminance\": " << this->iluminance << " }";
		return ss.str();
	}
}
