#pragma once

#include <memory>
#include <map>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

#include "io/universe_sender.hpp"
#include "filters/filter.hpp"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#include "LinearAllocator.h"
#pragma GCC diagnostic pop

#include "lib/logging.hpp"

namespace dmxfish::execution {

using scene_filter_vector_t = std::vector<std::shared_ptr<dmxfish::filters::filter>>;
using scene_boundry_vec_t = std::vector<size_t>;
using scene_filter_index_t = std::map<std::string, std::weak_ptr<dmxfish::filters::filter>>;
using default_dmx_value_container = std::vector<std::tuple<size_t, uint16_t, uint8_t>>;

class scene {
private:
	const scene_filter_vector_t filters;
	const scene_boundry_vec_t dependency_boundries;
	const std::shared_ptr<LinearAllocator> used_allocator;
	const scene_filter_index_t filter_index;
    const default_dmx_value_container default_dmx_values;
    const dmxfish::filters::channel_mapping output_channels;
public:
	scene (scene_filter_vector_t f, scene_boundry_vec_t b, std::shared_ptr<LinearAllocator> alloc,
           scene_filter_index_t index, default_dmx_value_container default_values,
           dmxfish::filters::channel_mapping output_channels_)
		: filters(std::move(f)), dependency_boundries(std::move(b)), used_allocator(alloc), filter_index(index),
        default_dmx_values(default_values), output_channels(std::move(output_channels_)) {
		// The vectors are initialized
	}

	scene() = delete;
	scene(const scene&) = delete;
	scene(scene&&) = default;

	/**
	 * Call this method in order to update all filter values.
	 */
	inline void invoke_filters() {
		// TODO evaluate single threaded vs multi threaded performance
		// Theoretically we can calculate results in [db_n, db_n+1)
		// in parallel but we might get in trouble with false
		// sharing (filters have different memory footprints)
		// as well as parallelization overhead.
		// -> Profiling required
		for(auto& filter : this->filters) {
			filter->update();
		}
	}

	/**
	 * This method gets called when the scene becomes visible.
	 */
	inline void on_start() {
        size_t i = 0;

        if (!this->default_dmx_values.empty()){
            auto uptr = dmxfish::io::get_universe(0);
            for (const auto& default_value: this->default_dmx_values) {
                const auto universe_id = std::get<0>(default_value);
                const auto channel = std::get<1>(default_value);
                const auto value = std::get<2>(default_value);
                if (universe_id != i) {
                    uptr = dmxfish::io::get_universe(universe_id);
                    i = universe_id;
                    if (uptr == nullptr) {
                        ::spdlog::error("Failed to update default DMX values for universe {}. Reason: the requested ID does not exist.", universe_id);
                    }
                }
                if (uptr != nullptr && channel < 512) {
                    (*uptr)[channel] = value;
                }
            }
            i = 0;
        }

		for(auto& filter: this->filters) {
			try {
				filter->scene_activated();
			} catch(std::exception& e) {
				::spdlog::error("Failed to perform scene_activated on filter {0:d}: {1}", i, e.what());
			}
			i++;
		}
	}

	/**
	 * This method gets called when the scene becomes invisible.
	 */
	inline void on_stop() {
        for (auto f_ptr : this->filters) {
            f_ptr->scene_deactivated();
        }
    }

	[[nodiscard]] inline size_t get_filter_count() {
		return this->filters.size();
	}

	[[nodiscard]] inline bool update_filter_parameter(const std::string& filter_id, const std::string& key, const std::string& value) {
		if(!this->filter_index.contains(filter_id)) {
			throw std::invalid_argument("The requested scene does not contain the requested filter '" + filter_id + "'.");
		}
		const auto fptr = this->filter_index.at(filter_id).lock();
		if(fptr) {
			return fptr->receive_update_from_gui(key, value);
		} else {
			return false;
		}
	}

	[[nodiscard]] inline std::optional<std::string> get_channel_value(const std::string& channel_id) const {
		if (const auto it = this->output_channels.eight_bit_channels.find(channel_id);
				it != this->output_channels.eight_bit_channels.end() && it->second != nullptr) {
			return std::to_string(static_cast<unsigned>(*it->second));
		}
		if (const auto it = this->output_channels.sixteen_bit_channels.find(channel_id);
				it != this->output_channels.sixteen_bit_channels.end() && it->second != nullptr) {
			return std::to_string(*it->second);
		}
		if (const auto it = this->output_channels.float_channels.find(channel_id);
				it != this->output_channels.float_channels.end() && it->second != nullptr) {
			return std::to_string(*it->second);
		}
		if (const auto it = this->output_channels.color_channels.find(channel_id);
				it != this->output_channels.color_channels.end() && it->second != nullptr) {
			auto* const p = it->second;
			std::stringstream ss;
			ss << p->getHue() << ',' << p->getSaturation() << ',' << p->getIluminance();
			return ss.str();
		}
		return std::nullopt;
	}
};

}
