#include "filters/filter_lua_script.hpp"

#include <string>

#include "lib/macros.hpp"
#include "lib/logging.hpp"
#include "dmx/pixel.hpp"
#include "lua/LuaCpp.hpp"
#include "lua/LuaContext.hpp"
#include "lua/LuaMetaObject.hpp"

#include <iostream>
//int count_occurence_of(const std::string &base_string, std::string pattern, size_t start, size_t end) {
//    int occurrences = 0;
//    while ((start = base_string.find(pattern, start)) != std::string::npos && start <= end) {
//        ++occurrences;
//        start += pattern.length();
//    }
//    return occurrences;
//}


class MetaMap : public LuaCpp::LuaMetaObject {
public:
    std::map<std::string, std::shared_ptr<LuaCpp::Engine::LuaType>> values;
    MetaMap() : values() {}
    bool inline Exists(const std::string &name) {
        return !(values.find( name ) == values.end());
    }

    std::shared_ptr<LuaCpp::Engine::LuaType> getValue(std::string &key) {
        if (Exists(key)) {
            return values[key];
        }
        return std::make_shared<LuaCpp::Engine::LuaTNil>();
    }

    void setValue(std::string &key, std::shared_ptr<LuaCpp::Engine::LuaType> val) {
        values[key] = val;
    }

};


class PixelLua : public LuaCpp::Engine::LuaType {
public:
    int a;
    PixelLua() : a() {}
    PixelLua(int b) : a(b) {}
    ~PixelLua() {}

    int getTypeId() const {
        return 101;
        return LUA_TNUMBER;
    }

    std::string getTypeName(LuaCpp::Engine::LuaState &L) const {
//        return std::string(lua_typename(L, 101));
        return std::string(lua_typename(L, LUA_TNUMBER));
    }

    void PushValue(LuaCpp::Engine::LuaState &L) {
        lua_pushnumber(L, a);
    }

    void PopValue(LuaCpp::Engine::LuaState &L, int idx) {
        if (lua_type(L, idx) == LUA_TNUMBER) {
            a = lua_tonumber(L,idx);
        } else {
            throw std::invalid_argument("The value at the stack position " + std::to_string(idx) + " is not LUA_TNUMBER");
        }
    }

    std::string ToString() const {
        return std::to_string(a);
    }


};

namespace dmxfish::filters {

    void filter_lua_script::pre_setup(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters) {
        MARK_UNUSED(initial_parameters);

        // Creage Lua context
        LuaCpp::LuaContext lua;

        lua.CompileString("test1", "print('get a ' .. foo) foo = 42 print('settet foo/a ' .. foo)");
//        lua.CompileString("test2", "print('foo[\"1\"] : ' .. foo[\"1\"] )");

        std::unique_ptr<LuaCpp::Engine::LuaState> L = lua.newStateFor("test1");

        PixelLua obj = PixelLua(3);

        obj.PushGlobal(*L, "foo");

        int res = lua_pcall(*L, 0, LUA_MULTRET, 0);
        if (res != LUA_OK ) {
            std::cout << "Error Executing " << res << " " << lua_tostring(*L,1) << "\n";
        }

        obj.PopGlobal(*L);

        std::cout << "After it has " << obj.ToString() << "\n";

//        L = lua.newStateFor("test2");
//        obj.PushGlobal(*L, "foo");
//
//        std::string atest = "\"1\"";
//        std::cout << obj.getValue(atest)->getTypeId() << "\n";
//
//
//        res = lua_pcall(*L, 0, LUA_MULTRET, 0);
//        if (res != LUA_OK ) {
//            std::cout << "Error Executing " << res << " " << lua_tostring(*L,1) << "\n";
//        }

//        // Creage Lua context
//        LuaCpp::LuaContext lua;
//
//        lua.CompileString("test1", "print('Assigning to foo[\"1\"] value \"testing MetaMap\"') foo[\"1\"] = 42");
//        lua.CompileString("test2", "print('foo[\"1\"] : ' .. foo[\"1\"] )");
//
//        std::unique_ptr<LuaCpp::Engine::LuaState> L = lua.newStateFor("test1");
//
//        MetaMap obj;
//
//        obj.PushGlobal(*L, "foo");
//
//        int res = lua_pcall(*L, 0, LUA_MULTRET, 0);
//        if (res != LUA_OK ) {
//            std::cout << "Error Executing " << res << " " << lua_tostring(*L,1) << "\n";
//        }
//
//        L = lua.newStateFor("test2");
//        obj.PushGlobal(*L, "foo");
//
//        std::string atest = "\"1\"";
//        std::cout << obj.getValue(atest)->getTypeId() << "\n";
//
//
//        res = lua_pcall(*L, 0, LUA_MULTRET, 0);
//        if (res != LUA_OK ) {
//            std::cout << "Error Executing " << res << " " << lua_tostring(*L,1) << "\n";
//        }

        if (!configuration.contains("mapping")) {
            throw filter_config_exception("cue filter: unable to setup the mapping");
        }

        std::string mapping = configuration.at("mapping");
        //::spdlog::debug("setup_filter: mapping: {}", mapping);
        size_t start_pos = 0;
        auto next_pos = mapping.find(";");
        
//	int count_channel_type = count_occurence_of(mapping, ":8bit", 0, mapping.size());
//        channel_names_eight.reserve(count_channel_type);
//        eight_bit_channels.reserve(count_channel_type);
//
//	count_channel_type = count_occurence_of(mapping, ":16bit", 0, mapping.size());
//        channel_names_sixteen.reserve(count_channel_type);
//        sixteen_bit_channels.reserve(count_channel_type);
//
//	count_channel_type = count_occurence_of(mapping, ":float", 0, mapping.size());
//        channel_names_float.reserve(count_channel_type);
//        float_channels.reserve(count_channel_type);
//
//	count_channel_type = count_occurence_of(mapping, ":color", 0, mapping.size());
//        channel_names_color.reserve(count_channel_type);
//        color_channels.reserve(count_channel_type);

        while (true) {
            const auto sign = mapping.find(":", start_pos);
            
            std::string channel_type = mapping.substr(sign + 1, next_pos - sign - 1);
            std::string channel_name = mapping.substr(start_pos, sign - start_pos);
            if (!channel_type.compare("8bit")) {
//                eight_bit_channels.push_back(channel_str(EIGHT_BIT, eight_bit_channels.size()));
                channel_names_eight.push_back(channel_name);
                eight_bit_channels.push_back(0);
            } else if (!channel_type.compare("16bit")) {
//                sixteen_bit_channels.push_back(channel_str(SIXTEEN_BIT, sixteen_bit_channels.size()));
                channel_names_sixteen.push_back(channel_name);
                sixteen_bit_channels.push_back(0);
            } else if (!channel_type.compare("float")) {
//                float_channel.push_back(channel_str(FLOAT, float_channels.size()));
                channel_names_float.push_back(channel_name);
                float_channels.push_back(0);
            } else if (!channel_type.compare("color")) {
//                color_channels.push_back(channel_str(COLOR, color_channels.size()));
                channel_names_color.push_back(channel_name);
                color_channels.push_back(dmxfish::dmx::pixel());
            } else {
                throw filter_config_exception(std::string("can not recognise channel type: ") + mapping.substr(sign + 1, next_pos - sign - 1));
            }

            if (next_pos >= mapping.length()) {
                break;
            }
            start_pos = next_pos + 1;
            next_pos = mapping.find(";", start_pos);
        }
    }

    void filter_lua_script::setup_filter(const std::map <std::string, std::string> &configuration,
                                  const std::map <std::string, std::string> &initial_parameters,
                                  const channel_mapping &input_channels) {
        MARK_UNUSED(initial_parameters);

        // Todo: update
        if (!configuration.contains("cuelist")) {
            throw filter_config_exception("cue filter: unable to setup the cuelist");
        }
    }

    bool filter_lua_script::receive_update_from_gui(const std::string &key, const std::string &_value) {

    }

    void filter_lua_script::get_output_channels(channel_mapping &map, const std::string &name) {
        for (size_t i = 0; i < eight_bit_channels.size(); i++) {
            map.eight_bit_channels[name + ":" + channel_names_eight.at(i)] = &eight_bit_channels.at(i);
        }
        for (size_t i = 0; i < sixteen_bit_channels.size(); i++) {
            map.sixteen_bit_channels[name + ":" + channel_names_sixteen.at(i)] = &sixteen_bit_channels.at(i);
        }
        for (size_t i = 0; i < float_channels.size(); i++) {
            map.float_channels[name + ":" + channel_names_float.at(i)] = &float_channels.at(i);
        }
        for (size_t i = 0; i < color_channels.size(); i++) {
            map.color_channels[name + ":" + channel_names_color.at(i)] = &color_channels.at(i);
        }
    }

    void filter_lua_script::update() {

    }

    void filter_lua_script::scene_activated() {
    }

}
