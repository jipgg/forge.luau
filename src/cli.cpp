#include "common.hpp"
#include <print>
#include <span>
#include <ranges>
#include <algorithm>
#include <string_view>
#include <optional>
namespace vws = std::views;
namespace rgs = std::ranges;

struct args_t {
    std::span<char*> data;
    auto view() const -> decltype(auto) {
        return vws::transform(data, [](auto v) -> std::string_view {return v;});
    }
    auto operator[](size_t idx) const -> std::optional<std::string_view> {
        if (idx >= data.size()) return std::nullopt;
        return data[idx];
    }
};

auto main(int argc, char** argv) -> int {
    auto args = args_t{.data = {argv, argv + argc}};
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
