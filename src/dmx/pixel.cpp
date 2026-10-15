//
// SPDX-License-Identifier: GPL-3.0-or-later
//
#include "dmx/pixel.hpp"

#include <algorithm>
#include <cmath>
#include <sstream>

namespace dmxfish::dmx {

    [[nodiscard]] std::string pixel::str() const {
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

    pixel::pixel (double h, double s, double i) : hue(h), saturation(s), iluminance(i), red(0), green(0), blue(0) {
	    convert_hsi_to_rgb_pre();
    }

    pixel::pixel (uint16_t r, uint16_t g, uint16_t b) : hue(0.), saturation(0.), iluminance(-1.), red(r), green(g), blue(b) {
	    convert_rgb_to_hsi_pre();
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
        double H = std::fmod(this->hue, 360.0);
        if (H < 0.0) H += 360.0;
        const double S = std::clamp(this->saturation, 0.0, 1.0);
        const double V = std::clamp(this->iluminance, 0.0, 1.0);

        const double C = V * S;
        const double H_prime = H / 60.0;
        const double X = C * (1.0 - std::abs(std::fmod(H_prime, 2.0) - 1.0));
        const double m = V - C;

        double R_prime, G_prime, B_prime;
        if (H_prime < 1.0)      { R_prime = C; G_prime = X; B_prime = 0.0; }
        else if (H_prime < 2.0) { R_prime = X; G_prime = C; B_prime = 0.0; }
        else if (H_prime < 3.0) { R_prime = 0.0; G_prime = C; B_prime = X; }
        else if (H_prime < 4.0) { R_prime = 0.0; G_prime = X; B_prime = C; }
        else if (H_prime < 5.0) { R_prime = X; G_prime = 0.0; B_prime = C; }
        else                    { R_prime = C; G_prime = 0.0; B_prime = X; }

        r = (uint16_t) std::round(65535.0 * (R_prime + m));
        g = (uint16_t) std::round(65535.0 * (G_prime + m));
        b = (uint16_t) std::round(65535.0 * (B_prime + m));
    }

    void pixel::pixel_to_rgbw(uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& w){
        // RGBW: the achromatic (white) portion V*(1-S) is routed to the W channel,
        // leaving the chroma C=V*S distributed across R/G/B per standard HSV sectors.
        double H = std::fmod(this->hue, 360.0);
        if (H < 0.0) H += 360.0;
        const double S = std::clamp(this->saturation, 0.0, 1.0);
        const double V = std::clamp(this->iluminance, 0.0, 1.0);

        const double C = V * S;
        const double H_prime = H / 60.0;
        const double X = C * (1.0 - std::abs(std::fmod(H_prime, 2.0) - 1.0));
        const double m = V - C;

        double R_prime, G_prime, B_prime;
        if (H_prime < 1.0)      { R_prime = C; G_prime = X; B_prime = 0.0; }
        else if (H_prime < 2.0) { R_prime = X; G_prime = C; B_prime = 0.0; }
        else if (H_prime < 3.0) { R_prime = 0.0; G_prime = C; B_prime = X; }
        else if (H_prime < 4.0) { R_prime = 0.0; G_prime = X; B_prime = C; }
        else if (H_prime < 5.0) { R_prime = X; G_prime = 0.0; B_prime = C; }
        else                    { R_prime = C; G_prime = 0.0; B_prime = X; }

        r = (uint8_t) std::round(255.0 * R_prime);
        g = (uint8_t) std::round(255.0 * G_prime);
        b = (uint8_t) std::round(255.0 * B_prime);
        w = (uint8_t) std::round(255.0 * m);
    }

    void pixel::convert_hsi_to_rgb(){
        pixel_to_rgb16(red, green, blue);
    }
    void pixel::convert_rgb_to_hsi(){
        const double r = this->red / 65535.0;
        const double g = this->green / 65535.0;
        const double b = this->blue / 65535.0;
        const double max_c = std::max({r, g, b});
        const double min_c = std::min({r, g, b});
        const double delta = max_c - min_c;

        this->iluminance = max_c;
        this->saturation = max_c > 0.0 ? delta / max_c : 0.0;

        if (delta <= 0.0) {
            this->hue = 0.0;
            return;
        }

        double H;
        if (max_c == r) {
            H = 60.0 * std::fmod(((g - b) / delta) + 6.0, 6.0);
        } else if (max_c == g) {
            H = 60.0 * (((b - r) / delta) + 2.0);
        } else {
            H = 60.0 * (((r - g) / delta) + 4.0);
        }
        this->hue = H;
    }

    void pixel::convert_hsi_to_rgb_pre(){
        if (this->red == 0 and this->green == 0 and this->blue == 0 and this->iluminance > 0.){
            convert_hsi_to_rgb();
        }
    }
    void pixel::convert_rgb_to_hsi_pre(){
        if (this->iluminance < -0.5){
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
	convert_hsi_to_rgb_pre();
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
	convert_hsi_to_rgb_pre();
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
	convert_hsi_to_rgb_pre();
    }

    void pixel::setRed(uint16_t r){
        convert_hsi_to_rgb_pre();
        invalidate_hsi();
        this->red = r;
	convert_rgb_to_hsi_pre();
    }

    void pixel::setGreen(uint16_t g){
        convert_hsi_to_rgb_pre();
        invalidate_hsi();
        this->green = g;
	convert_rgb_to_hsi_pre();
    }

    void pixel::setBlue(uint16_t b){
        convert_hsi_to_rgb_pre();
        invalidate_hsi();
        this->blue = b;
	convert_rgb_to_hsi_pre();
    }

    dmxfish::dmx::pixel mix_color_interleaving(dmxfish::dmx::pixel c1, dmxfish::dmx::pixel c2, double range) {
        if (range < 0.0 || range > 1.0) {
            throw std::invalid_argument("The range interval needs to be within 0 and 1.");
        }

        dmxfish::dmx::pixel output;
        const double h1 = c1.getHue();
        const double h2 = c2.getHue();
	const double s1 = c1.getSaturation();
	const double s2 = c2.getSaturation();
	const double i1 = c1.getIluminance();
	const double i2 = c2.getIluminance();

        const auto hue_diff = std::fmod(h1-h2 + 180.0 + 360.0, (double) 360.0) - ((double) 180.0);
        output.setHue(std::fmod(360.0 + h2 + ((hue_diff*(range*2.0))/2.0), (double) 360.0));
        output.setSaturation((s1 * (range)) + (s2 * (1.0-range)));
        output.setIluminance((i1 * range) + (i2 * (1.0-range)));

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

namespace std {
    std::string to_string(const dmxfish::dmx::pixel &p) {
        return p.str();
    }
}
