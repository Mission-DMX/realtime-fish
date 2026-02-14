//
// Created by leondietrich on 2/11/25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#include "filters/lua/lua_scene_api.hpp"

#include <stdexcept>
#include <string>
#include <utility>

#include <sol/sol.hpp>

#include "main.hpp"

namespace dmxfish::filters::lua {
    size_t get_scene_count() {
        if (auto io_mgr_ptr = get_iomanager_instance(); io_mgr_ptr != nullptr) [[likely]] {
            if (auto show_ptr = io_mgr_ptr->get_active_show(); show_ptr != nullptr) [[likely]] {
                return show_ptr->get_scene_count();
            } else {
                return 0;
            }
        } else {
            return 0;
        }
    }

    size_t get_current_active_scene() {
        if (auto io_mgr_ptr = get_iomanager_instance(); io_mgr_ptr != nullptr) [[likely]] {
            if (auto show_ptr = io_mgr_ptr->get_active_show(); show_ptr != nullptr) [[likely]] {
                return show_ptr->get_active_scene_index();
            } else {
                return 0;
            }
        } else {
            return 0;
        }
    }

    size_t get_current_active_scene_id() {
        if (auto io_mgr_ptr = get_iomanager_instance(); io_mgr_ptr != nullptr) [[likely]] {
            if (auto show_ptr = io_mgr_ptr->get_active_show(); show_ptr != nullptr) [[likely]] {
                return show_ptr->find_scene_id_from_index(show_ptr->get_active_scene_index());
            } else {
                return 0;
            }
        } else {
            return 0;
        }
    }

    size_t get_default_scene_id() {
        if (auto io_mgr_ptr = get_iomanager_instance(); io_mgr_ptr != nullptr) [[likely]] {
            if (auto show_ptr = io_mgr_ptr->get_active_show(); show_ptr != nullptr) [[likely]] {
                return show_ptr->get_default_scene();
            } else {
                return 0;
            }
        } else {
            return 0;
        }
    }

    bool switch_to_scene(size_t new_scene_id) {
        if (auto io_mgr_ptr = get_iomanager_instance(); io_mgr_ptr != nullptr) [[likely]] {
            if (auto show_ptr = io_mgr_ptr->get_active_show(); show_ptr != nullptr) [[likely]] {
                return show_ptr->set_active_scene(new_scene_id);
            } else {
                return false;
            }
        } else {
            return false;
        }
	}

    void init_lua_scene_api(sol::state& lua) {
        lua.set_function("get_scene_count",
		    dmxfish::filters::lua::get_scene_count
        );
        lua.set_function("get_own_scene_index",
            dmxfish::filters::lua::get_current_active_scene
        );
        lua.set_function("get_own_scene_id",
            dmxfish::filters::lua::get_current_active_scene_id
        );
	lua.set_function("switch_to_scene",
            dmxfish::filters::lua::switch_to_scene
        );
        lua.set_function("get_default_scene_id",
            dmxfish::filters::lua::get_default_scene_id
        );
    }
}
