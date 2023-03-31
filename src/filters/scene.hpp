#pragma once

#include <memory>
#include <vector>

#include "filters/filter.hpp"
#include "LinearAllocator.h"

namespace dmxfish::filters {

// TODO find a way to change shared_ptr to unique_ptr as this scene is the only object requiring ownership

using scene_filter_vector_t = std::vector<std::shared_ptr<filter>>;
using scene_boundry_vec_t = std::vector<size_t>;

class scene {
private:
	const scene_filter_vector_t filters;
	const scene_boundry_vec_t dependency_boundries;
	const std::shared_ptr<LinearAllocator> used_allocator;
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
		for(auto& filter : filters) {
			filter->update();
		}
	}

	/**
	 * This method gets called when the scene becomes visible.
	 */
	void on_start() {
	}

	/**
	 * This method gets called when the scene becomes invisible.
	 */
	void on_stop() {}
};

}
