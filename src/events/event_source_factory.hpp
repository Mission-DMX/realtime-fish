//
// Created by doralitze on 3/31/25.
// SPDX-License-Identifier: GPL-3.0-or-later
//
#pragma once

#include "lib/macros.hpp"
COMPILER_SUPRESS("-Wuseless-cast")
#include "proto_src/Events.pb.h"
COMPILER_RESTORE("-Wuseless-cast")

namespace dmxfish::events {
    [[nodiscard]] bool construct_or_update_event_source_from_message(const missiondmx::fish::ipcmessages::event_sender& msg);
}
