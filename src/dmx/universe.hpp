#pragma once

#include <cstdint>
#include <iterator>
#include <ranges>

#define DMX_UNIVERSE_SIZE 512

namespace dmxfish::dmx {

	typedef uint8_t channel_8bit_t;
	//typedef std::iterator<std::contiguous_iterator_tag, channel_8bit_t> universe_iterator;
	typedef channel_8bit_t* universe_iterator;

	enum class universe_type : uint8_t {
		PHYSICAL = 0,
		ARTNET = 1,
		sACN = 2,
		FTDI = 3,
	};

	class universe {
	private:
		const int id;
		const universe_type type;
    protected:
        bool _is_dummy;
	public:
        /**
         * Construct a new universe
         *
         * @param _id The ID of the universe
         * @param _type The type of universe
         * @param is_dummy Should physical output be enabled?
         */
		universe(const int _id, const universe_type _type, const bool is_dummy) : id(_id), type(_type), _is_dummy(is_dummy) {}

        /**
         * Get the ID of the universe.
         */
		[[nodiscard]] inline int getID() const {
			return this->id;
		}

        /**
         * Get the connection type of this universe.
         */
		[[nodiscard]] inline universe_type getUniverseType() const {
			return this->type;
		}

        /**
         * Is the universe output disabled?
         *
         * This has no effect regarding requested DMX values by a connected GUI.
         *
         * @return True if no physical output is desired.
         */
        [[nodiscard]] inline bool is_dummy() const {
            return this->_is_dummy;
        }

        /**
         * Enable or disable output state.
         *
         * @param enabled Enables dummy mode (which disables output).
         */
        inline void set_dummy_mode(const bool enabled) {
            this->_is_dummy = enabled;
        }

		virtual channel_8bit_t& operator[](size_t p) = 0;
		virtual universe_iterator begin() = 0;
		virtual universe_iterator end() = 0;
	};
}
