#include "common.hpp"

static auto collect(lua_State* L) -> int {
    lua_gc(L, LUA_GCCOLLECT, 0);
    return lua::none;
} 
static auto count(lua_State* L) -> int {
    return lua::push(L, lua_gc(L, LUA_GCCOUNT, 0));
} 

void push_garbage(lua_State* L) {
    constexpr auto lib = std::to_array<luaL_Reg>({
        {"collect", collect},
        {"count", count},
    });
    push_library(L, lib);
}
