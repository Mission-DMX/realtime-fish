#pragma once

#include <cstdint>
#include <string>

namespace dmxfish::dmx {

struct rgb_output_pixel {
	uint8_t red;
	uint8_t green;
	uint8_t blue;

	rgb_output_pixel() : red(0), green(0), blue(0) {}
	rgb_output_pixel(uint8_t r, uint8_t g, uint8_t b) : red(r), green(g), blue(b) {}
};

struct rgbwau_output_pixel : public rgb_output_pixel {
	uint8_t white;
	uint8_t amber;
	uint8_t uv;

	rgbwau_output_pixel() : rgb_output_pixel(), white(0), amber(0), uv(0) {}
	rgbwau_output_pixel(uint8_t r, uint8_t g, uint8_t b, uint8_t w, uint8_t a, uint8_t u) : rgb_output_pixel(r,g,b), white(w), amber(a), uv(u) {}
};

class pixel {
    private:
	double hue;
	double saturation;
	double iluminance;  // iluminance < 0 -> hsi value is invalid, (when getting h,s or i, it gets calculated)
    uint16_t red;
    uint16_t green;
    uint16_t blue;  // iluminance > 0 and r,b,n = 0 -> rgb value is invalid, (when getting r,g or b it gets calculated)
    void convert_hsi_to_rgb();
    void convert_rgb_to_hsi();
    inline void convert_hsi_to_rgb_pre();
    inline void convert_rgb_to_hsi_pre();
    inline void invalidate_rgb();
    inline void invalidate_hsi();
    void pixel_to_rgb16(uint16_t& r, uint16_t& g, uint16_t& b);

    public:
	pixel () : hue(0.), saturation(0.), iluminance(0.), red(0), green(0), blue(0) {}
    pixel (double h, double s, double i) : hue(h), saturation(s), iluminance(i), red(0), green(0), blue(0) {}
    pixel (uint16_t r, uint16_t g, uint16_t b) : hue(0.), saturation(0.), iluminance(-1.), red(r), green(g), blue(b) {}

    void pixel_to_rgb(uint8_t& r, uint8_t& g, uint8_t& b);
    void pixel_to_rgbw(uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& w);

    [[nodiscard]] double getHue();
    [[nodiscard]] double getSaturation();
    [[nodiscard]] double getIluminance();
    [[nodiscard]] uint16_t getRed();
    [[nodiscard]] uint16_t getGreen();
    [[nodiscard]] uint16_t getBlue();

    void setHue(double h);
    void setSaturation(double s);
    void setIluminance(double i);
    void setRed(uint16_t r);
    void setGreen(uint16_t g);
    void setBlue(uint16_t b);

	[[nodiscard]] std::string str();
};

}
