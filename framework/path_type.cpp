#include "builtin.hpp"
#include <format>
#include <filesystem>
#include "lua.hpp"
#include "userdata.hpp"
register_type_name(path_t, "filesystem_path");

static auto namecall(state_t L) -> int {
    auto& self = userdata::self<path_t>(L);
    const auto [atom, name] = lua::namecall_atom<Method>(L);
    using m_t = Method;
    switch (atom) {
        case m_t::string:
            return lua::push(L, self);
        case m_t::extension:
            return lua::push(L, self.extension());
        case m_t::has_extension:
            return lua::push(L, self.has_extension());
        case m_t::replace_extension:
            return push_path(L, self.replace_extension(luaL_checkstring(L, 2)));
        case m_t::parent_path:
            return push_path(L, self.parent_path());
        case m_t::is_absolute:
            return lua::push(L, self.is_absolute());
        case m_t::is_relative:
            return lua::push(L, self.is_relative());
        case m_t::filename:
            return lua::push(L, self.filename());
        case m_t::has_filename:
            return lua::push(L, self.has_filename());
        case m_t::replace_filename:
            return push_path(L, self.replace_filename(luaL_checkstring(L, 2)));
        case m_t::remove_filename:
            return push_path(L, self.remove_filename());
        default:
            break;
    }
    luaL_errorL(L, "invalid namecall '%s'", name);
}
static auto tostring(state_t L) -> int {
    auto fmt = std::format(
        "{}: \"{}\"",
        userdata::name_for<path_t>(),
        userdata::self<path_t>(L).string());
    lua_pushstring(L, fmt.c_str());
    return 1;
}
static auto div(state_t L) -> int { return userdata::push(L, to_path(L, 1) / to_path(L, 2));
}

void register_path(state_t L) {
    auto name = userdata::name_for<path_t>();
    if (luaL_newmetatable(L, name)) {
        const luaL_Reg metatable[] = {
            {"__namecall", namecall},
            {"__tostring", tostring},
            {"__div", div},
            {nullptr, nullptr}
        };
        luaL_register(L, nullptr, metatable);
        lua_pushstring(L, name);
        lua_setfield(L, -2, "__type");
    }
    lua_pop(L, 1);
    userdata::set_dtor<path_t>(L);
}
auto push_path(state_t L, const path_t& path) -> int {
    return userdata::push(L, path);
}
auto to_path(state_t L, int idx) -> path_t {
    return userdata::or_else<path_t>(L, idx, [&]{return luaL_checkstring(L, idx);});
}
