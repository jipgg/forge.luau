#ifndef LUA_HPP
#define LUA_HPP
#include <concepts>
#include <utility>
#include <string_view>
#include <string>
#include <cstddef>
#include "lualib.h"
namespace lua {
using state_t = lua_State*;
constexpr auto none = 0;
struct nil_t{};
constexpr nil_t nil{};
constexpr auto push(state_t L, std::string_view v) -> int {
    lua_pushlstring(L, v.data(), v.size());
    return 1;
}
constexpr auto push(state_t L, const std::string& v) -> int {
    lua_pushstring(L, v.c_str());
    return 1;
}
constexpr auto push(state_t L, nil_t nil) -> int {
    lua_pushnil(L);
    return 1;
}
constexpr auto push(state_t L, lua_CFunction fn) -> int {
    lua_pushcfunction(L, fn, "anonymous");
    return 1;
}
constexpr auto push(state_t L, bool boolean) -> int {
    lua_pushboolean(L, boolean);
    return 1;
} 
constexpr auto push(state_t L, double v) -> int {
    lua_pushnumber(L, v);
    return 1;
}
constexpr auto push(state_t L, int v) -> int {
    lua_pushinteger(L, v);
    return 1;
}
template <class Ty>
concept Pushable = requires (state_t L, Ty&& v) {
    ::lua::push(L, std::forward<Ty>(v));
};
template <Pushable ...Tys>
constexpr auto push_range(state_t L, Tys&&...args) -> int {
    (push(L, std::forward<Tys>(args)),...);
    return sizeof...(args);
}
template <class From, class To>
concept Castable_to = requires {
    static_cast<From>(To{});
};
template<Castable_to<int> Ty = int>
constexpr auto namecall_atom(state_t L) -> std::pair<Ty, const char*> {
    int atom{};
    const char* name = lua_namecallatom(L, &atom);
    return {static_cast<Ty>(atom), name};
}
namespace detail {
template <class Ty>
void default_dtor(void* ud) {
    static_cast<Ty*>(ud)->~Ty();
}
}
template <class Ty,  class ...Params>
requires std::constructible_from<Ty, Params&&...>
constexpr auto make_userdata(state_t L, Params&&...args) -> Ty& {
    auto dtor = [](auto ud) {static_cast<Ty*>(ud)->~Ty();};
    Ty* ud = static_cast<Ty*>(lua_newuserdatadtor(L, sizeof(Ty), dtor));
    std::construct_at(ud, std::forward<Params>(args)...);
    return *ud;
}
template <class Ty>
constexpr auto new_userdata(state_t L, Ty&& userdata) {
    return make_userdata<Ty>(L, std::forward<Ty>(userdata));
}
template <class Ty>
constexpr auto to_userdata(state_t L, int idx) -> Ty& {
    return *static_cast<Ty*>(lua_touserdata(L, idx));
}

}
#endif
