//
// Created by Leon Dietrich on 05.02.25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#include <array>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <string>

#include "filters/filter_color_mixer.hpp"

using namespace dmxfish::filters;
using namespace dmxfish::dmx;

void print_color(const pixel& p) {
    std::cout << "{\"r\":" << p.getRed();
    std::cout << ", \"g\":" << p.getGreen();
    std::cout << ", \"b\":" << p.getBlue();
    std::cout << "}";
}

void print_filter_results(const std::string& filter_name, int method) {
    std::cout << "\n\t\"" << filter_name << "\":[\n";

    std::array<std::array<std::optional<pixel>, 3>, 4> test_cases = {
            {pixel(60.0, 1.0, 1.0), pixel(350.0, 1.0, 1.0), std::nullopt},
            {pixel(60.0, 1.0, 1.0), pixel(240.0, 1.0, 1.0), std::nullopt},
            {pixel(240.0, 0.5, 1.0), pixel(0.0, 0.0, 1.0), std::nullopt},
            {pixel(0.0, 1.0, 1.0), pixel(120.0, 1.0, 1.0), pixel(240.0, 1.0, 1.0)}
    };

    for (auto i = 0, j = 0; i < test_cases.size();) {
        std::cout << "\t\t{\"inputs\":[";
        for (j = 0; j < test_cases[i].size(); j++) {
            if (!test_cases[i][j].has_value()) {
                continue;
            }
            if (j > 0) {
                std::cout << ", ";
            }
            print_color(test_cases[i][j].value());
        }

        std::shared_ptr<filter> cmf;
        switch(method) {
            case 0:
                cmf = std::make_shared<filter_color_mixer_hsv>();
                break;
            case 1:
                cmf = std::make_shared<filter_color_mixer_add_rgb>();
                break;
            case 2:
                cmf = std::make_shared<filter_color_mixer_norm_rgb>();
                break;
            default:
                std::cerr << "Implementation bug: method " << method << " not implemented." << std::endl;
                exit(1);
        }

        channel_mapping input_channels, output_channels;
        std::map<std::string, std::string> configuration, initial_parameters;
        configuration["input_count"] = std::to_string(j);
        for (int k = 0; k <= j; k++) {
            input_channels.color_channels[std::to_string(k)] = &(test_cases[i][k].value());
        }
        cmf->setup_filter(configuration, initial_parameters, input_channels, "test_filter");
        cmf->get_output_channels(output_channels, "test_filter");
        cmf->scene_activated();
        cmf.update();

        std::cout << "], \"output\":" << print_color(output_channels.color_channels["test_filter:value"]) << "}";
        if (++i < test_cases.size()) {
            std::cout << ",";
        }
        std::cout << "\n";
    }

    std::cout << "]";
}

int main(int argc, char* argv[]) {
    MARK_UNUSED(argc);
    MARK_UNUSED(argv);

    std::cout << "{";

    print_filter_results("hsv", 0);
    std::cout << ",";
    print_filter_results("additive_rgb", 1);
    std::cout << ",";
    print_filter_results("normative_rgb", 2);

    std::cout << "}" << std::endl;

    return 0;
}