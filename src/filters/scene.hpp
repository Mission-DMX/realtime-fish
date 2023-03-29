#pragma once

#include <memory>
#include <vector>

#include "filters/filter.hpp"

namespace dmxfish::filters {

class scene {
private:
	std::vector<std::unique_ptr<filter>> filters;
public:
	scene () : filters{} {
	}

	/**
	 * Call this method in order to update all filter values.
	 */
	inline void invoke_filters() {
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
