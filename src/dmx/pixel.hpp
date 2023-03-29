#pragma once

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

struct pixel {
	double hue;
	double saturation;
	double value;

	pixel () : hue(0.), saturation(0.), value(0.) {}
	pixel (double h, double s, double v) : hue(h), saturation(s), value(v) {}
};

}
