#include "dmx/pixel.hpp"

#include <sstream>
#include "cmath"

namespace dmxfish::dmx {

    [[nodiscard]] std::string pixel::str() {
		std::stringstream ss;
        ss << "{\"hue\": " << this->hue << ", \"saturation\": " << this->saturation << ", \"iluminance\": " << this->iluminance << " }";
		return ss.str();
	}

    void pixel::pixel_to_rgb(uint8_t& r, uint8_t& g, uint8_t& b){
        double H = this->hue, S = this->saturation, I = this->iluminance;
        H = std::fmod(H, 360);
        H = 3.14159*H / (double) 180;
        S = S>0 ? (S<1 ? S : 1) : 0;
        I = I>0 ? (I<1 ? I : 1) : 0;

        if(H < 2.09439) {
            r = (uint8_t) std::round(255*I/3*(1+S*std::cos(H)/std::cos(1.047196667-H)));
            g = (uint8_t) std::round(255*I/3*(1+S*(1-std::cos(H)/std::cos(1.047196667-H))));
            b = (uint8_t) std::round(255*I/3*(1-S));
        } else if(H < 4.188787) {
            H = H - 2.09439;
            g = (uint8_t) std::round(255*I/3*(1+S*std::cos(H)/std::cos(1.047196667-H)));
            b = (uint8_t) std::round(255*I/3*(1+S*(1-std::cos(H)/std::cos(1.047196667-H))));
            r = (uint8_t) std::round(255*I/3*(1-S));
        } else {
            H = H - 4.188787;
            b = (uint8_t) std::round(255*I/3*(1+S*std::cos(H)/std::cos(1.047196667-H)));
            r = (uint8_t) std::round(255*I/3*(1+S*(1-std::cos(H)/std::cos(1.047196667-H))));
            g = (uint8_t) std::round(255*I/3*(1-S));
        }
    }

    void pixel::pixel_to_rgbw(uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& w){
        double H = this->hue, S = this->saturation, I = this->iluminance;
        double cos_h, cos_1047_h;

        H = std::fmod(H,360);
        H = 3.14159*H/(double)180;
        S = S>0 ? (S<1 ? S : 1) : 0;
        I = I>0 ? (I<1 ? I : 1) : 0;

        if(H < 2.09439) {
            cos_h = std::cos(H);
            cos_1047_h = std::cos(1.047196667-H);
            r = (uint8_t) std::round(S*255*I / 3*(1+cos_h/cos_1047_h));
            g = (uint8_t) std::round(S*255*I / 3*(1+(1-cos_h/cos_1047_h)));
            b = 0;
            w = (uint8_t) std::round(255*(1-S)*I);
        } else if(H < 4.188787) {
            H = H - 2.09439;
            cos_h = std::cos(H);
            cos_1047_h = std::cos(1.047196667-H);
            g = (uint8_t) std::round(S*255*I / 3*(1+cos_h/cos_1047_h));
            b = (uint8_t) std::round(S*255*I / 3*(1+(1-cos_h/cos_1047_h)));
            r = 0;
            w = (uint8_t) std::round(255*(1-S)*I);
        } else {
            H = H - 4.188787;
            cos_h = std::cos(H);
            cos_1047_h = std::cos(1.047196667-H);
            b = (uint8_t) std::round(S*255*I / 3*(1+cos_h/cos_1047_h));
            r = (uint8_t) std::round(S*255*I / 3*(1+(1-cos_h/cos_1047_h)));
            g = 0;
            w = (uint8_t) std::round(255*(1-S)*I);
        }
    }

}
