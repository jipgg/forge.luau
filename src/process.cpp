#include "common.hpp"
#include <cstdlib>

using arg_iterator_t = decltype(get_args().data.begin());

static auto system(state_t L) -> int {
    int exit_code = ::system(luaL_checkstring(L, 1));
    return luau::push(L, exit_code);
}
static auto arg_iterator_closure(state_t L) -> int {
    auto& it = luau::to_userdata<arg_iterator_t>(L, lua_upvalueindex(1));
    if (it == get_args().data.end()) return luau::none;
    lua_pushstring(L, *it);
    ++it;
    return 1;
}
static auto arg_iterator(state_t L) -> int {
    auto args = get_args().data.begin();
    luau::make_userdata<arg_iterator_t>(L, get_args().data.begin());
    lua_pushcclosure(L, arg_iterator_closure, "arg_iterator", 1);
    return 1;
}

void open_process(state_t L, library_config config) {
    luaL_Reg const process[] = {
        {"system", system},
        {"arg_iterator", arg_iterator},
        {nullptr, nullptr}
    };
    config.apply(L, process);
}
