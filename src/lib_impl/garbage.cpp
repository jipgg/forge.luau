#include "common.hpp"

static auto collect(state_t L) -> int {
    lua_gc(L, LUA_GCCOLLECT, 0);
    return luau::none;
} 
static auto count(state_t L) -> int {
    return luau::push(L, lua_gc(L, LUA_GCCOUNT, 0));
} 

template<>
void garbage::open(state_t L, library_config config) {
    const luaL_Reg lib[] = {
        {"collect", collect},
        {"count", count},
        {nullptr, nullptr}
    };
    config.apply(L, lib);
}
