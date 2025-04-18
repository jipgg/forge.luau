#include "common.hpp"
#include <print>
#include <optional>

static args_wrapper args;


auto main(int argc, char** argv) -> int {
    args = args_wrapper{argc, argv};
    auto state = setup_state();
    auto L = state.get();

    auto expected = load_script(L, args[1].value_or("init.luau"));
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
auto get_args() -> args_wrapper const& {
    return args;
}
