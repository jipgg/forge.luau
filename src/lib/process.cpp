#include "common.hpp"
#include <cstdlib>
#include <chrono>
#include <thread>
using sec_t = std::chrono::duration<double, std::ratio<1>>;

using arg_iterator_t = decltype(internal::get_args().data.begin());

static auto system(state_t L) -> int {
    int exit_code = ::system(luaL_checkstring(L, 1));
    return luau::push(L, exit_code);
}
static auto arg_iterator_closure(state_t L) -> int {
    auto& it = luau::to_userdata<arg_iterator_t>(L, lua_upvalueindex(1));
    if (it == internal::get_args().data.end()) return luau::none;
    lua_pushstring(L, *it);
    ++it;
    return 1;
}
static auto args(state_t L) -> int {
    auto const& wrapper = internal::get_args();
    lua_createtable(L, wrapper.data.size(), 0);
    int i{1};
    for (auto arg : wrapper.view()) {
        lua_pushlstring(L, arg.data(), arg.size());
        lua_rawseti(L, -2, i++);
    }
    return 1;
}
static auto arg_iterator(state_t L) -> int {
    auto args = internal::get_args().data.begin();
    luau::make_userdata<arg_iterator_t>(L, internal::get_args().data.begin());
    lua_pushcclosure(L, arg_iterator_closure, "arg_iterator", 1);
    return 1;
}
static auto sleep_for(state_t L) -> int {
    std::this_thread::sleep_for(sec_t{luaL_checknumber(L, 1)});
    return luau::none;
}
template<>
void process::open(state_t L, library_config config) {
    luaL_Reg const process[] = {
        {"system", system},
        {"args", args},
        {"arg_iterator", arg_iterator},
        {"sleep_for", sleep_for},
        {nullptr, nullptr}
    };
    config.apply(L, process);
}
