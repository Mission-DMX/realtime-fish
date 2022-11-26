#pragma once

#include <array>
#include <cstdint>

#define DMX_UNIVERSE_SIZE 512

namespace dmxfish::dmx {

	typedef uint8_t channel_8bit_t;

	struct universe {
		const int id;
		std::array<channel_8bit_t, DMX_UNIVERSE_SIZE> data;

		universe(int id) : id(id), data{} {}
	};
}
