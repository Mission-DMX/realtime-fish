#pragma once

#include <cstdint>
#include <iterator>
#include <ranges>

#define DMX_UNIVERSE_SIZE 512

namespace dmxfish::dmx {

	typedef uint8_t channel_8bit_t;
	typedef std::iterator<contiguous_iterator_tag, channel_8bit_t> universe_iterator;

	enum class universe_type : uint8_t {
		PHYSICAL = 0,
		ARTNET = 1
	};

	class universe {
	private:
		const int id;
		const universe_type type;
	public:
		universe(const int id, const universe_type type) : id(id), type(type) {}

		const int getID() const {
			return this->id;
		}

		const universe_type getUniverseType() const {
			return this->type;
		}

		virtual channel_8bit_t& operator[](size_t p) = 0;
		virtual universe_iterator begin() = 0;
		virtual universe_iterator end() = 0;
	};
}
