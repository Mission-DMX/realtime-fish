#pragma once

#include <forward_list>
#include <memory>

#include "dmx/universe.hpp"

#include "proto_src/UniverseControl.pb.h"

namespace dmxfish::io {

	// TODO write method that creates/updates all universes from show file and removes the unessessary ones from active_universes registry

	/**
	 * Push the values of the provided universe
	 * @param universe The universe of which data is to be pushed
	 * @return True if the update was successfully pushed
	 */
	bool publish_universe_update(std::shared_ptr<dmxfish::dmx::universe> universe);

	/**
	 * Push updates of all active primary universes.
	 * @return True if no house keeping was required.
	 */
	bool push_all_registered_universes();

	/**
	 * Use this method to unregister (and potentially delete) a universe.
	 * @param id The ID of the universe to unregister
	 */
	void unregister_universe(const int id);

	/**
	 * Create a temporary universe that won't be registered with the active ones.
	 * @param output_description The address to send to
	 * @return A shared pointer to the universe
	 */
	std::shared_ptr<dmxfish::dmx::universe> get_temporary_universe(const std::string& output_description);

	/**
	 * Create or update a universe from a given IPC message.
	 * @param u The universe definition to use
	 */
	std::shared_ptr<dmxfish::dmx::universe> register_universe_from_message(const missiondmx::fish::ipcmessages::Universe& u);

	/**
	 * Get a list of all registered universes.
	 */
	std::forward_list<std::weak_ptr<dmxfish::dmx::universe>> get_universe_list();
}
