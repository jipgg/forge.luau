#include "common.hpp"
#include <cstdlib>
#include <chrono>
#include <thread>
using sec_t = std::chrono::duration<double, std::ratio<1>>;

using arg_iterator_t = decltype(internal::get_args().span().begin());

static auto system(lua_State* L) -> int {
    int exit_code = ::system(luaL_checkstring(L, 1));
    return lua::push(L, exit_code);
}
static auto arg_iterator_closure(lua_State* L) -> int {
    auto& it = lua::to_userdata<arg_iterator_t>(L, lua_upvalueindex(1));
    if (it == internal::get_args().span().end()) return lua::none;
    lua_pushstring(L, *it);
    ++it;
    return 1;
}
static auto args(lua_State* L) -> int {
    auto args = internal::get_args().span().begin();
    lua::make_userdata<arg_iterator_t>(L, internal::get_args().span().begin());
    lua_pushcclosure(L, arg_iterator_closure, "arg_iterator", 1);
    return 1;
}
static auto sleep(lua_State* L) -> int {
    std::this_thread::sleep_for(sec_t{luaL_checknumber(L, 1)});
    return lua::none;
}
void open_oslib(lua_State* L) {
    const luaL_Reg extra[] = {
        {"system", system},
        {"args", args},
        {"sleep", sleep},
        {nullptr, nullptr}
    };
    lua_getglobal(L, "os");
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        luaopen_os(L);
        lua_getglobal(L, "os");
    }
    luaL_register(L, nullptr, extra);
}
