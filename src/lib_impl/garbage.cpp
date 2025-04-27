#include "common.hpp"

static auto collect(state_t L) -> int {
    lua_gc(L, LUA_GCCOLLECT, 0);
    return luau::none;
} 
static auto count(state_t L) -> int {
    return luau::push(L, lua_gc(L, LUA_GCCOUNT, 0));
} 

template<>
auto lib::garbage::name() -> std::string {return "garbage";}
template<>
void lib::garbage::push(state_t L) {
    constexpr auto lib = std::to_array<luaL_Reg>({
        {"collect", collect},
        {"count", count},
    });
    push_api(L, lib);
}
