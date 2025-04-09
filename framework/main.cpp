#include <print>
#include <lualib.h>
#include "framework.hpp"
constexpr auto builtin_library_name = "bin";

auto main() -> int {
    auto state = fw::setup_state();
    auto L = state.get();
    lua_newtable(L);
    filesystem::get(L);
    json::get(L);
    fileio::get(L);
    lua_setglobal(L, builtin_library_name);
    luaL_sandbox(L);
    auto expected = fw::load_script(state.get(), "abc.luau");
    if (!expected) {
        std::println("\033[35mError: {}\033[0m", expected.error());
        return -1;
    }
    auto status = lua_resume(*expected, state.get(), 0);
    if (status != LUA_OK) {
        std::println("\033[35mError: {}\033[0m", luaL_checkstring(*expected, -1));
    }
    return 0;
}

