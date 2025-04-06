#ifndef FRAMEWORK_HPP
#define FRAMEWORK_HPP

#include <memory>
#include <expected>
#include <filesystem>
#include <lualib.h>
#include <concepts>
#include "compile_time.hpp"
using path_t = std::filesystem::path;
using state_t = lua_State*;
using state_owner_t = std::unique_ptr<lua_State, decltype(&lua_close)>;

enum class Method {
    read_all,
    children,
    COMPILE_TIME_ENUM_SENTINEL
};

namespace fw {
auto setup_state() -> state_owner_t;
auto load_script(state_t L, const path_t& path) -> std::expected<state_t, std::string>;
}
namespace filesystem {
void open(state_t);
}

namespace fw {
constexpr auto push_api(state_t L, const luaL_Reg* api) {
    lua_newtable(L);
    luaL_register(L, nullptr, api);
}
constexpr auto set_api(state_t L, const luaL_Reg* api, const std::string& name, int idx = -1) {
    lua_newtable(L);
    luaL_register(L, nullptr, api);
}
constexpr auto to_method(state_t L) -> Method {
    int atom;
    lua_namecallatom(L, &atom);
    return static_cast<Method>(atom);
}
template <class From, class To>
concept Castable_To = requires {
    static_cast<From>(To{});
};
template <class Ty>
constexpr auto as_userdata(state_t L, int idx) -> Ty& {
    return *static_cast<Ty*>(lua_touserdata(L, idx));
}
namespace detail {
template <class Ty>
void default_dtor(void* ud) {
    static_cast<Ty*>(ud)->~Ty();
}
}
template <class Ty,  class ...Params>
requires std::constructible_from<Ty, Params&&...>
auto make_userdata(lua_State* L, Params&&...args) -> Ty& {
    Ty* ud = static_cast<Ty*>(lua_newuserdatadtor(L, sizeof(Ty), detail::default_dtor<Ty>));
    std::construct_at(ud, std::forward<Params>(args)...);
    return *ud;
}
}

#endif
