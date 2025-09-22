//
// Created by Doralitze on 11.05.25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#pragma once

#include <string>

#include "events/event.hpp"

namespace dmxfish::events {

    event_sender_t parse_sender_representation(const std::string &s);

}