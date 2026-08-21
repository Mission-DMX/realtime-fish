#include "control_desk/device_enumeration.hpp"

#include <filesystem>
#include <fstream>
#include <iterator>
#include <map>

#include "lib/logging.hpp"
#include "utils.hpp"

namespace dmxfish::control_desk {

    std::list<std::pair<std::string, midi_device_id>> enumerate_control_devices() {
        std::list<std::pair<std::string, midi_device_id>> l;

        std::map<std::string, midi_device_id> identified_ids;
        {
                std::ifstream file("/proc/asound/cards");
                std::string line;
                if (file.is_open()) {
                    while (std::getline(file, line)) {
                        auto parts = utils::split(line, ':');
                        if (parts.size() == 2) {
                            parts = utils::split(parts.front(), '[');
                            const auto card_id = std::stoi(utils::trim(parts.front()));
                            parts.pop_front();
                            std::string device_type = utils::str_replace(parts.front(), "]", "");
                            utils::rtrim(device_type);
                            const auto path = "/dev/midi" + std::to_string(card_id);
                            if (device_type == "XTouch") {
                                ::spdlog::debug("Identified Xtouch extension card id {} -> {} as {}", card_id, device_type, path);
                                identified_ids[path] = midi_device_id::X_TOUCH;
                            } else if(device_type == "XTouchExt") {
                                ::spdlog::debug("Identified Xtouch Extender extension card id {} -> {} as {}", card_id, device_type, path);
                                identified_ids[path] = midi_device_id::X_TOUCH_EXTENSION;
                            }
                        }
                    }
                    file.close();
                } else {
                    ::spdlog::error("Expected ALSA kernel module to be loaded an exposing card registry.");
                }

        }

        for (const auto& entry : std::filesystem::directory_iterator("/dev/")) {
            if(auto p = std::string(entry.path()); p.rfind("/dev/midi", 0) == 0) {
                auto identified_id = midi_device_id::X_TOUCH;
                if (identified_ids.contains(p)) {
                    identified_id = identified_ids.at(p);
                }
                if (identified_id == midi_device_id::X_TOUCH_EXTENSION) {
                    l.push_front(std::make_pair(p, identified_id));
                } else {
                    l.push_back(std::make_pair(p, identified_id));
                }
            }
        }
        return l;
    }

}
