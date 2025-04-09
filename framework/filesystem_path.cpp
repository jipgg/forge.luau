#include "framework.hpp"
#include <format>
namespace cm = common;

static auto namecall(state_t L) -> int {
    auto& self = cm::to_userdata_tagged<filesystem::path_t, Type::fs_path>(L, 1);
    const auto [atom, name] = cm::namecall_atom<Method>(L);
    using m_t = Method;
    switch (atom) {
        case m_t::string:
            return fw::as_string(L, self.string());
        case m_t::extension:
            return fw::as_string(L, self.extension());
        case m_t::has_extension:
            return fw::as_boolean(L, self.has_extension());
        case m_t::replace_extension:
            return fw::as_path(L, self.replace_extension(luaL_checkstring(L, 2)));
        case m_t::parent_path:
            return fw::as_path(L, self.parent_path());
        case m_t::is_absolute:
            return fw::as_boolean(L, self.is_absolute());
        case m_t::is_relative:
            return fw::as_boolean(L, self.is_relative());
        case m_t::filename:
            return fw::as_string(L, self.filename());
        case m_t::has_filename:
            return fw::as_boolean(L, self.has_filename());
        case m_t::replace_filename:
            return fw::as_path(L, self.replace_filename(luaL_checkstring(L, 2)));
        case m_t::remove_filename:
            return fw::as_path(L, self.remove_filename());
        default:
            break;
    }
    luaL_errorL(L, "invalid namecall '%s'", name);
}
static auto tostring(state_t L) -> int {
    auto fmt = std::format("{}: \"{}\"", filesystem::path_tname, fw::to_path(L, 1).string());
    lua_pushstring(L, fmt.c_str());
    return 1;
}
static auto div(state_t L) -> int {
    return fw::as_path(L, fw::check_path(L, 1) / fw::check_path(L, 2));
}

void filesystem::push_path(state_t L, const path_t& value) {
    common::make_userdata_tagged<path_t, Type::fs_path>(L, value);
    if (luaL_newmetatable(L, path_tname)) {
        const luaL_Reg metatable[] = {
            {"__namecall", namecall},
            {"__tostring", tostring},
            {"__div", div},
            {nullptr, nullptr}
        };
        luaL_register(L, nullptr, metatable);
        lua_pushstring(L, path_tname);
        lua_setfield(L, -2, "__type");
    }
    lua_setmetatable(L, -2);
}
