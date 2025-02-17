#include "dmx/pixel.hpp"

#include <sstream>
#include "cmath"
#include "pixel.hpp"


namespace dmxfish::dmx {

    [[nodiscard]] std::string pixel::str() {
        std::stringstream sshsi;
        std::stringstream ssrgb;
        std::stringstream ss;
        sshsi << "{\"hue\": " << this->hue << ", \"saturation\": " << this->saturation << ", \"iluminance\": "
           << this->iluminance << " } ";
        ssrgb << "{\"red\": " << this->red << ", \"green\": " << this->green << ", \"blue\": " << this->blue << " }";
        if (this->iluminance < 0){
            ss << ssrgb.str() << "; hsi not valid: ";
        }
        ss << sshsi.str();
        if (this->red == 0 and this->green == 0 and this->blue == 0 and this->iluminance > 0.) {
            ss << "; rgb not valid: ";
        }
        if (this->iluminance >= 0){
            ss << ssrgb.str();
        }

		return ss.str();
	}

    void pixel::pixel_to_rgb(uint8_t& r, uint8_t& g, uint8_t& b){
        uint16_t r16;
        uint16_t g16;
        uint16_t b16;
        pixel_to_rgb16(r16, g16, b16);
        r=(uint8_t) ((r16 & 0xFF00) >> 8);
        g=(uint8_t) ((g16 & 0xFF00) >> 8);
        b=(uint8_t) ((b16 & 0xFF00) >> 8);
    }

    void pixel::pixel_to_rgb16(uint16_t& r, uint16_t& g, uint16_t& b){
        double H = this->hue, S = this->saturation, I = this->iluminance;
        H = std::fmod(H, 360);
        H = 3.14159*H / (double) 180;
        S = S>0 ? (S<1 ? S : 1) : 0;
        I = I>0 ? (I<1 ? I : 1) : 0;

        if(H < 2.09439) {
            r = (uint16_t) std::round(65535.0*I/3*(1+S*std::cos(H)/std::cos(1.047196667-H)));
            g = (uint16_t) std::round(65535.0*I/3*(1+S*(1-std::cos(H)/std::cos(1.047196667-H))));
            b = (uint16_t) std::round(65535.0*I/3*(1-S));
        } else if(H < 4.188787) {
            H = H - 2.09439;
            g = (uint16_t) std::round(65535.0*I/3*(1+S*std::cos(H)/std::cos(1.047196667-H)));
            b = (uint16_t) std::round(65535.0*I/3*(1+S*(1-std::cos(H)/std::cos(1.047196667-H))));
            r = (uint16_t) std::round(65535.0*I/3*(1-S));
        } else {
            H = H - 4.188787;
            b = (uint16_t) std::round(65535.0*I/3*(1+S*std::cos(H)/std::cos(1.047196667-H)));
            r = (uint16_t) std::round(65535.0*I/3*(1+S*(1-std::cos(H)/std::cos(1.047196667-H))));
            g = (uint16_t) std::round(65535.0*I/3*(1-S));
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

    void pixel::convert_hsi_to_rgb(){
        pixel_to_rgb16(red, green, blue);
    }
    void pixel::convert_rgb_to_hsi(){
        double r = this->red;
        double g = this->green;
        double b = this->blue;
        double sum = r + g + b;
        if (sum > 65535.0){
            r = r * 65535.0 / sum;
            g = g * 65535.0 / sum;
            b = b * 65535.0 / sum;
            sum = r + g + b;
        }
        if (sum <= 0.0){
            this->hue = 0.0;
            this->saturation = 0.0;
            this->iluminance = 0.0;
            return;
        }
        double min = std::min(std::min(r, g), b);
        this->saturation = 1.0 - (min * 3.0 / sum);
        this->iluminance = sum / 65535.0;

        if (this->saturation <= 0.0){
            this->hue = 0.0;
            return;
        }
        // double hue_1 = -1.0;
        //https://www.wolframalpha.com/input?i=solve+%5B%2F%2Fmath%3Acos%28x%29%2F%28cos%28pi%2F3-x%29%29%3Da%2F%2F%5D+for+%5B%2F%2Fmath%3Ax%2F%2F%5D
        if (b <= 0.0){
            double a = (3 * r / (65535.0 * this->iluminance)- 1) * (1/this->saturation);
            // hue_1 = 2 * (std::atan((std::sqrt(3) * a - 2 * std::sqrt(a * a - a + 1)) / (a - 2))) *180.0/3.14159;
            this->hue = std::fmod(2 * (std::atan((std::sqrt(3) * a + 2 * std::sqrt(a * a - a + 1)) / (a - 2))) *180.0/3.14159 + 180.0, 360.0);
        }
        else if (r <= 0.0){
            double a = (3 * g / (65535.0 * this->iluminance)- 1) * (1/this->saturation);
            // hue_1 = 2 * (std::atan((std::sqrt(3) * a - 2 * std::sqrt(a * a - a + 1)) / (a - 2))) *180.0/3.14159 + 120.0;
            this->hue = std::fmod(2 * (std::atan((std::sqrt(3) * a + 2 * std::sqrt(a * a - a + 1)) / (a - 2))) *180.0/3.14159 + 300.0, 360.0);
        }
        else if (g <= 0.0){
            double a = (3 * b / (65535.0 * this->iluminance)- 1) * (1/this->saturation);
            // hue_1 = 2 * (std::atan((std::sqrt(3) * a - 2 * std::sqrt(a * a - a + 1)) / (a - 2))) *180.0/3.14159 + 240.0;
            this->hue = std::fmod(2 * (std::atan((std::sqrt(3) * a + 2 * std::sqrt(a * a - a + 1)) / (a - 2))) *180.0/3.14159 + 420.0, 360.0);
        }
    }
    void pixel::convert_hsi_to_rgb_pre(){
        if (this->red == 0 and this->green == 0 and this->blue == 0 and this->iluminance > 0.){
            convert_hsi_to_rgb();
        }
    }
    void pixel::convert_rgb_to_hsi_pre(){
        if (this->iluminance < 0.){
            convert_rgb_to_hsi();
        }
    }
    void pixel::invalidate_rgb(){
        this->red = 0;
        this->green = 0;
        this->blue = 0;
    }
    void pixel::invalidate_hsi(){
        this->iluminance = -1.;
    }

    double pixel::getHue(){
        convert_rgb_to_hsi_pre();
        return this->hue;
    }
    double pixel::getSaturation(){
        convert_rgb_to_hsi_pre();
        return this->saturation;
    }
    double pixel::getIluminance(){
        convert_rgb_to_hsi_pre();
        return this->iluminance;
    }
    uint16_t pixel::getRed(){
        convert_hsi_to_rgb_pre();
        return this->red;
    }
    uint16_t pixel::getGreen(){
        convert_hsi_to_rgb_pre();
        return this->green;
    }
    uint16_t pixel::getBlue(){
        convert_hsi_to_rgb_pre();
        return this->blue;
    }

    void pixel::setHue(double h){
        convert_rgb_to_hsi_pre();
        invalidate_rgb();
        this->hue = h;
    }

    void pixel::setSaturation(double s){
        convert_rgb_to_hsi_pre();
        invalidate_rgb();
	if (s > 1.0) {
		s = 1.0;
	} else if (s < 0.0) {
		s = 0.0;
	}
        this->saturation = s;
    }

    void pixel::setIluminance(double i){
        convert_rgb_to_hsi_pre();
        invalidate_rgb();
        if (i > 1.0) {
		i = 1.0;
	} else if (i < 0.0) {
		i = 0.0;
	}
	this->iluminance = i;
    }

    void pixel::setRed(uint16_t r){
        convert_hsi_to_rgb_pre();
        invalidate_hsi();
        this->red = r;
    }

    void pixel::setGreen(uint16_t g){
        convert_hsi_to_rgb_pre();
        invalidate_hsi();
        this->green = g;
    }

    void pixel::setBlue(uint16_t b){
        convert_hsi_to_rgb_pre();
        invalidate_hsi();
        this->blue = b;
    }

    dmxfish::dmx::pixel mix_color_interleaving(dmxfish::dmx::pixel c1, dmxfish::dmx::pixel c2, double range) {
        if (range < 0.0 || range > 1.0) {
            throw std::invalid_argument("The range interval needs to be within 0 and 1.");
        }

        dmxfish::dmx::pixel output;
        const double h1 = c1.getHue();
        const double h2 = c2.getHue();

        const auto hue_diff = std::fmod(h1-h2 + 180.0 + 360.0, (double) 360.0) - ((double) 180.0);
        output.setHue(std::fmod(360.0 + h2 + ((hue_diff*(range*2.0))/2.0), (double) 360.0));
        output.setSaturation((c1.getSaturation() * (range)) + (c2.getSaturation() * (1.0-range)));
        output.setIluminance((c1.getIluminance() * range) + (c2.getIluminance() * (1.0-range)));

        return output;
    }

    dmxfish::dmx::pixel stopixel(const std::string& str) {
        const auto first_position = str.find(",");
        const auto h = std::stod(str.substr(0, first_position));
        const auto second_position = str.find(",", first_position + 1);
        const auto s = std::stod(str.substr(first_position + 1, second_position - first_position - 1));
        const auto i = std::stod(str.substr(second_position + 1));
        return {h, s, i};
    }
}
