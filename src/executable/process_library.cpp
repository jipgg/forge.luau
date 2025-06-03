#include "common.hpp"
#include "util/lua.hpp"
#include <cstdlib>
#include <chrono>
#include <thread>
#include <print>
#include <array>
#include <ranges>
#include <format>
using sec_t = std::chrono::duration<double, std::ratio<1>>;
namespace vws = std::views;
namespace rgs = std::ranges;
using std::vector;
using std::string;

static auto system(lua_State* L) -> int {
    int exit_code = ::system(luaL_checkstring(L, 1));
    return lua::push(L, exit_code);
}
static auto sleep(lua_State* L) -> int {
    std::this_thread::sleep_for(sec_t{luaL_checknumber(L, 1)});
    return lua::none;
}
void loader::process(lua_State* L, int idx) {
    lua::set_functions(L, idx, std::to_array<luaL_Reg>({
        {"system", system},
        {"sleep", sleep},
    }));
}
