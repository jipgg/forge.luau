#pragma once
#include <nlohmann/json.hpp>
#include <lualib.h>

namespace util {
static auto is_table_arraylike(lua_State* L, int index) -> bool {
    index = lua_absindex(L, index);
    lua_Integer expected_index = 1;
    lua_pushnil(L);
    while (lua_next(L, index)) {
        if (lua_type(L, -2) != LUA_TNUMBER or lua_tointeger(L, -2) != expected_index) {
            lua_pop(L, 2);
            return false;
        }
        expected_index++;
        lua_pop(L, 1);
    }
    return true;
}
static auto table_to_json(lua_State* L, int idx) -> nlohmann::json {
    nlohmann::json json;
    idx = lua_absindex(L, idx);
    if (is_table_arraylike(L, idx)) {
        json = nlohmann::json::array();
        lua_pushnil(L);
        while (lua_next(L, idx) != 0) {
            int type = lua_type(L, -1);
            switch (type) {
                case LUA_TSTRING:
                    json.push_back(lua_tostring(L, -1));
                    break;
                case LUA_TNUMBER:
                    json.push_back(lua_tonumber(L, -1));
                    break;
                case LUA_TBOOLEAN:
                    json.push_back(lua_toboolean(L, -1));
                    break;
                case LUA_TTABLE:
                    json.push_back(table_to_json(L, lua_gettop(L)));
                    break;
            }
            lua_pop(L, 1); // Pop value
        }
    } else {
        lua_pushnil(L);
        while (lua_next(L, idx) != 0) {
            std::string key = lua_tostring(L, -2);
            int type = lua_type(L, -1);
            switch (type) {
                case LUA_TSTRING:
                    json[key] = lua_tostring(L, -1);
                    break;
                case LUA_TNUMBER:
                    json[key] = lua_tonumber(L, -1);
                    break;
                case LUA_TBOOLEAN:
                    json[key] = lua_toboolean(L, -1);
                    break;
                case LUA_TTABLE:
                    json[key] = table_to_json(L, lua_gettop(L));
                    break;
            }
            lua_pop(L, 1);
        }
    }
    return json;
}
static auto push_value(lua_State* L,  const nlohmann::basic_json<>& val) -> int {
    using val_t = nlohmann::json::value_t;
    switch (val.type()) {
        case val_t::string:
            lua_pushstring(L, std::string(val).c_str());
        return 1;
        case val_t::boolean:
            lua_pushboolean(L, bool(val));
        return 1;
        case val_t::null:
            lua_pushnil(L);
        return 1;
        case val_t::number_float:
        case val_t::number_integer:
        case val_t::number_unsigned:
            lua_pushnumber(L, double(val));
        return 1;
        case val_t::binary: {
            const auto& vec = val.get_binary();
            uint8_t* buf = static_cast<uint8_t*>(lua_newbuffer(L, vec.size()));
            std::memcpy(buf, vec.data(), vec.size());
        } return 1;
        case val_t::discarded:
            luaL_errorL(L, "discarded value");
        break;
        case val_t::array: {
            lua_newtable(L);
            int idx{1};
            for (const auto& subval : val) {
                push_value(L, subval);
                lua_rawseti(L, -2, idx++);
            }
        } return 1;
        case val_t::object: {
            lua_newtable(L);
            for (const auto& [key, subval] : val.items()) {
                push_value(L, subval);
                lua_rawsetfield(L, -2, std::string(key).c_str());
            }
        } return 1;
    }
    return 0;
}
static auto push_json_as_table(lua_State* L, std::string_view json) {
    try {
        auto parsed = nlohmann::json::parse(json);
        push_value(L, parsed);
        return 1;
    } catch (nlohmann::json::exception& e) {
        luaL_errorL(L, "%s", e.what());
    }
}
}
