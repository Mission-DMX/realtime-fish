#pragma once

#include <memory>
#include <vector>

#include "filters/filter.hpp"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#include "LinearAllocator.h"
#pragma GCC diagnostic pop

namespace dmxfish::execution {

using scene_filter_vector_t = std::vector<std::shared_ptr<dmxfish::filters::filter>>;
using scene_boundry_vec_t = std::vector<size_t>;

class scene {
private:
	const scene_filter_vector_t filters;
	const scene_boundry_vec_t dependency_boundries;
	const std::shared_ptr<LinearAllocator> used_allocator;
	// TODO introduce std::map<std::string, std::shared_ptr<dmxfish::filters::filter>> for gui parameter updates
public:
	scene (scene_filter_vector_t f, scene_boundry_vec_t b, std::shared_ptr<LinearAllocator> alloc)
		: filters(std::move(f)), dependency_boundries(std::move(b)), used_allocator(alloc) {
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
		for(auto& filter: this->filters) {
			filter->scene_activated();
		}
	}

	/**
	 * This method gets called when the scene becomes invisible.
	 */
	inline void on_stop() {}

	[[nodiscard]] inline size_t get_filter_count() {
		return this->filters.size();
	}
};

}
