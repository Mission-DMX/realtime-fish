//
// Created by leondietrich on 2/11/25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#include "filters/lua/lua_color_api.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <string>
#include <utility>

#include <sol/sol.hpp>

#include "dmx/pixel.hpp"
#include "lib/macros.hpp"

namespace dmxfish::filters::lua {

    dmxfish::dmx::pixel table2pixel(sol::table& t) {
        if (auto h = t.get<sol::optional<double>>("h"), s = t.get<sol::optional<double>>("s"),
                    i = t.get<sol::optional<double>>("i"); h && s && i) {
            return dmxfish::dmx::pixel(h.value(), s.value(), i.value());
        } else if (auto r = t.get<sol::optional<double>>("r"), g = t.get<sol::optional<double>>("g"),
                    b = t.get<sol::optional<double>>("b"); r && g && b) {
            dmxfish::dmx::pixel p;
            p.setRed(r.value());
            p.setGreen(g.value());
            p.setBlue(b.value());
            return p;
        } else {
            throw std::invalid_argument("Cannot convert table to color. Expected a table containing hsi or rgb values.");
        }
    }

    std::tuple<uint8_t, uint8_t, uint8_t> hsi_to_rgb_color(dmxfish::dmx::pixel& color){
        uint8_t r = 0;
        uint8_t g = 0;
        uint8_t b = 0;
        color.pixel_to_rgb(r, g, b);
        return std::make_tuple(r, g, b);
    }

    std::tuple<uint8_t, uint8_t, uint8_t> hsi_to_rgb_table(sol::table color){
        dmxfish::dmx::pixel color_local = table2pixel(color);
        return hsi_to_rgb_color(color_local);
    }

    std::tuple<uint8_t, uint8_t, uint8_t, uint8_t> hsi_to_rgbw_color(dmxfish::dmx::pixel& color){
        uint8_t r = 0;
        uint8_t g = 0;
        uint8_t b = 0;
        uint8_t w = 0;
        color.pixel_to_rgbw(r, g, b, w);
        return std::make_tuple(r, g, b, w);
    }

    std::tuple<uint8_t, uint8_t, uint8_t, uint8_t> hsi_to_rgbw_table(sol::table color){
        dmxfish::dmx::pixel color_local = table2pixel(color);
        return hsi_to_rgbw_color(color_local);
    }

    inline sol::table pixel2table(dmxfish::dmx::pixel output) {
        sol::table output_color;
        output_color["h"] = output.getHue();
        output_color["s"] = output.getSaturation();
        output_color["i"] = output.getIluminance();

        return output_color;
    }

    dmxfish::dmx::pixel mix_color_rgb_additive_cc(dmxfish::dmx::pixel c1, dmxfish::dmx::pixel c2) {
        dmxfish::dmx::pixel output;

        const double r = c1.getRed() + c2.getRed();
        const double g = c1.getGreen() + c2.getGreen();
        const double b = c1.getBlue() + c2.getBlue();

        output.setRed(r > 65535 ? 65535 : r);
        output.setGreen(g > 65535 ? 65535 : g);
        output.setBlue(b > 65535 ? 65535 : b);

        return output;
    };

    dmxfish::dmx::pixel mix_color_rgb_additive_ct(dmxfish::dmx::pixel c1, sol::table color2) {
        dmxfish::dmx::pixel c2 = table2pixel(color2);
        return mix_color_rgb_additive_cc(c1, c2);
    }

    dmxfish::dmx::pixel mix_color_rgb_additive_tc(sol::table color1, dmxfish::dmx::pixel c2) {
        dmxfish::dmx::pixel c1 = table2pixel(color1);
        return mix_color_rgb_additive_cc(c1, c2);
    }

    dmxfish::dmx::pixel mix_color_rgb_additive_tt(sol::table color1, sol::table color2) {
        dmxfish::dmx::pixel c1 = table2pixel(color1);
        dmxfish::dmx::pixel c2 = table2pixel(color2);
        return mix_color_rgb_additive_cc(c1, c2);
    }

    dmxfish::dmx::pixel mix_color_rgb_normative_cc(dmxfish::dmx::pixel c1, dmxfish::dmx::pixel c2) {
        dmxfish::dmx::pixel output;

        const double c1r = c1.getRed();
        const double c2r = c2.getRed();
        const double c1g = c1.getGreen();
        const double c2g = c2.getGreen();
        const double c1b = c1.getBlue();
        const double c2b = c2.getBlue();

        const double r = std::sqrt(c1r*c1r + c2r*c2r);
        const double g = std::sqrt(c1g*c1g + c2g*c2g);
        const double b = std::sqrt(c1b*c1b + c2b*c2b);
        const double i = std::min(std::max(c1.getIluminance(), c2.getIluminance()), 1.0);

        const auto vlen = std::sqrt(r*r+g*g+b*b);
        output.setRed((r/vlen)*65535*i);
        output.setGreen((g/vlen)*65535*i);
        output.setBlue((b/vlen)*65535*i);

        return output;
    }

    dmxfish::dmx::pixel mix_color_rgb_normative_ct(dmxfish::dmx::pixel c1, sol::table color2) {
        dmxfish::dmx::pixel c2 = table2pixel(color2);
        return mix_color_rgb_normative_cc(c1, c2);
    }

    dmxfish::dmx::pixel mix_color_rgb_normative_tc(sol::table color1, dmxfish::dmx::pixel c2) {
        dmxfish::dmx::pixel c1 = table2pixel(color1);
        return mix_color_rgb_normative_cc(c1, c2);
    }

    dmxfish::dmx::pixel mix_color_rgb_normative_tt(sol::table color1, sol::table color2) {
        dmxfish::dmx::pixel c1 = table2pixel(color1);
        dmxfish::dmx::pixel c2 = table2pixel(color2);
        return mix_color_rgb_normative_cc(c1, c2);
    }

    dmxfish::dmx::pixel mix_color_hsi_ccb(dmxfish::dmx::pixel c1, dmxfish::dmx::pixel c2, bool reduce_saturation_on_far_angles) {
        dmxfish::dmx::pixel output;
        const double h1 = c1.getHue();
        const double h2 = c2.getHue();

        const auto hue_diff = std::fmod(h1-h2 + 180.0 + 360.0, (double) 360.0) - ((double) 180.0);
        output.setHue(std::fmod(360.0 + h2 + (hue_diff/2.0), (double) 360.0));
        const double saturation_reduction = (reduce_saturation_on_far_angles && hue_diff > 45.0) ? hue_diff / 360.0 : 0.0;
        output.setSaturation((c1.getSaturation() + c2.getSaturation() - saturation_reduction) / 2.0);
        output.setIluminance((c1.getIluminance() + c2.getIluminance() + (saturation_reduction / 2.0)) / 2.0);

        return output;
    }

    dmxfish::dmx::pixel mix_color_hsi_ctb(dmxfish::dmx::pixel c1, sol::table color2, bool reduce_saturation_on_far_angles) {
        dmxfish::dmx::pixel c2 = table2pixel(color2);
        return mix_color_hsi_ccb(c1, c2, reduce_saturation_on_far_angles);
    }

    dmxfish::dmx::pixel mix_color_hsi_tcb(sol::table color1, dmxfish::dmx::pixel c2, bool reduce_saturation_on_far_angles) {
        dmxfish::dmx::pixel c1 = table2pixel(color1);
        return mix_color_hsi_ccb(c1, c2, reduce_saturation_on_far_angles);
    }

    dmxfish::dmx::pixel mix_color_hsi_ttb(sol::table color1, sol::table color2, bool reduce_saturation_on_far_angles) {
        dmxfish::dmx::pixel c1 = table2pixel(color1);
        dmxfish::dmx::pixel c2 = table2pixel(color2);
        return mix_color_hsi_ccb(c1, c2, reduce_saturation_on_far_angles);
    }

    dmxfish::dmx::pixel mix_color_hsi_cc(dmxfish::dmx::pixel c1, dmxfish::dmx::pixel c2) {
        return mix_color_hsi_ccb(c1, c2, true);
    }

    dmxfish::dmx::pixel mix_color_hsi_ct(dmxfish::dmx::pixel c1, sol::table color2) {
        return mix_color_hsi_ctb(c1, color2, true);
    }

    dmxfish::dmx::pixel mix_color_hsi_tc(sol::table color1, dmxfish::dmx::pixel c2) {
        return mix_color_hsi_tcb(color1, c2, true);
    }

    dmxfish::dmx::pixel mix_color_hsi_tt(sol::table color1, sol::table color2) {
        return mix_color_hsi_ttb(color1, color2, true);
    }

    dmxfish::dmx::pixel mix_color_interleaving_cc(dmxfish::dmx::pixel c1, dmxfish::dmx::pixel c2, double range) {
        if (range < 0.0 || range > 1.0) {
            throw std::invalid_argument("The range interval needs to be within 0 and 1.");
        }

        dmxfish::dmx::pixel output;
        const double h1 = c1.getHue();
        const double h2 = c2.getHue();

        const auto hue_diff = std::fmod(h1-h2 + 180.0 + 360.0, (double) 360.0) - ((double) 180.0);
        output.setHue(std::fmod(360.0 + h2 + ((hue_diff*(range*2.0))/2.0), (double) 360.0));
        output.setSaturation((c1.getSaturation() * (range)) + (c2.getSaturation() * (1.0-range)));
        output.setIluminance((c1.getIluminance() * range) + (c2.getIluminance() * (1.0-range)));

        return output;
    }

    dmxfish::dmx::pixel mix_color_interleaving_ct(dmxfish::dmx::pixel c1, sol::table color2, double r) {
        dmxfish::dmx::pixel c2 = table2pixel(color2);
        return mix_color_interleaving_cc(c1, c2, r);
    }

    dmxfish::dmx::pixel mix_color_interleaving_tc(sol::table color1, dmxfish::dmx::pixel c2, double r) {
        dmxfish::dmx::pixel c1 = table2pixel(color1);
        return mix_color_interleaving_cc(c1, c2, r);
    }

    dmxfish::dmx::pixel mix_color_interleaving_tt(sol::table color1, sol::table color2, double r) {
        dmxfish::dmx::pixel c1 = table2pixel(color1);
        dmxfish::dmx::pixel c2 = table2pixel(color2);
        return mix_color_interleaving_cc(c1, c2, r);
    }

    void init_lua_color_api(sol::state& lua) {
        lua.set_function( "hsi_to_rgb", sol::overload(
                dmxfish::filters::lua::hsi_to_rgb_color,
                dmxfish::filters::lua::hsi_to_rgb_table
        ) );

        lua.set_function( "hsi_to_rgbw", sol::overload(
                dmxfish::filters::lua::hsi_to_rgbw_color,
                dmxfish::filters::lua::hsi_to_rgbw_table
        ) );

        lua.set_function("mix_color_rgb_additive", sol::overload(
                dmxfish::filters::lua::mix_color_rgb_additive_cc,
                dmxfish::filters::lua::mix_color_rgb_additive_ct,
                dmxfish::filters::lua::mix_color_rgb_additive_tc,
                dmxfish::filters::lua::mix_color_rgb_additive_tt
        ));

        lua.set_function("mix_color_rgb_normative", sol::overload(
                dmxfish::filters::lua::mix_color_rgb_normative_cc,
                dmxfish::filters::lua::mix_color_rgb_normative_ct,
                dmxfish::filters::lua::mix_color_rgb_normative_tc,
                dmxfish::filters::lua::mix_color_rgb_normative_tt
        ));

        lua.set_function("mix_color_hsi", sol::overload(
                dmxfish::filters::lua::mix_color_hsi_cc,
                dmxfish::filters::lua::mix_color_hsi_ct,
                dmxfish::filters::lua::mix_color_hsi_tc,
                dmxfish::filters::lua::mix_color_hsi_tt,
                dmxfish::filters::lua::mix_color_hsi_ccb,
                dmxfish::filters::lua::mix_color_hsi_ctb,
                dmxfish::filters::lua::mix_color_hsi_tcb,
                dmxfish::filters::lua::mix_color_hsi_ttb
        ));

        lua.set_function("mix_color_interleaving", sol::overload(
                dmxfish::filters::lua::mix_color_interleaving_cc,
                dmxfish::filters::lua::mix_color_interleaving_ct,
                dmxfish::filters::lua::mix_color_interleaving_tc,
                dmxfish::filters::lua::mix_color_interleaving_tt
        ));
    }
}