#include "common.hpp"
#include <nlohmann/json.hpp>

using json_t = nlohmann::json;
using json_value_t = decltype(json_t::parse(""));

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
static auto table_to_json(lua_State* L, int idx) -> json_t {
    json_t json;
    idx = lua_absindex(L, idx);
    if (is_table_arraylike(L, idx)) {
        json = json_t::array();
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

static auto push_value(lua_State* L,  const json_value_t& val) -> int {
    using val_t = json_t::value_t;
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
static auto decode(lua_State* L) -> int {
    try {
        auto parsed = json_t::parse(luaL_checkstring(L, 1));
        push_value(L, parsed);
        return 1;
    } catch (json_t::exception& e) {
        luaL_errorL(L, "%s", e.what());
    }
}
static auto encode(lua_State* L) -> int {
    return lua::push(L, table_to_json(L, 1).dump());
}
void loader::json(lua_State* L, int idx) {
    lua::set_functions(L, idx, std::to_array<luaL_Reg>({
        {"encode", encode},
        {"decode", decode},
    }));
}
// void open_jsonlib(lua_State* L) {
//     const luaL_Reg json[] = {
//         {"encode", encode},
//         {"decode", decode},
//         {nullptr, nullptr}
//     };
//     luaL_register(L, "json", json);
// }
