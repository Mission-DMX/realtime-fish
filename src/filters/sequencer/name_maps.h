//
// Created by Leon Dietrich on 18.02.25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#pragma once

namespace dmxfish::filters {
    struct name_maps {
        std::map<std::string, size_t> name_to_id_8bit, name_to_id_16bit, name_to_id_float, name_to_id_color;
        name_maps() : name_to_id_8bit(), name_to_id_16bit(), name_to_id_float(), name_to_id_color() {}
    };
}