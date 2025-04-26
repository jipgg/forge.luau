#include "common.hpp"
#include <print>
#include <optional>
#include <ranges>
#include <filesystem>
namespace vws = std::views;
namespace fs = std::filesystem;
using namespace std::string_view_literals;
std::string runCode(lua_State* L, const std::string& source);

static args_wrapper args;

static auto run_script(state_t L, std::string_view script) -> void {
    auto expected = load_script(L, script);
    if (!expected) {
        std::println("\033[35mError: {}\033[0m", expected.error());
        return;
    }
    auto status = lua_resume(*expected, L, 0);
    if (status != LUA_OK) {
        std::println("\033[35mError: {}\033[0m", luaL_checkstring(*expected, -1));
    }
}

auto main(int argc, char** argv) -> int {
    std::println("{}", fs::current_path().generic_string());
    args = args_wrapper{argc, argv};
    auto state = init_state();
    auto L = state.get();
    auto scripts = vws::filter([](std::string_view e) {
        return e.ends_with(".luau");
    });
    for (auto script : args.view() | scripts) {
        constexpr auto wildcard = "*.luau"sv;
        if (script.ends_with(wildcard)) {
            auto dir = fs::path(script).parent_path();
            for (auto entry : fs::directory_iterator(dir)) {
                auto const path = entry.path();
                if (not entry.is_regular_file() or path.extension() != ".luau") {
                    continue;
                }
                run_script(L, path.string());
            }
        } else {
            run_script(L, script);
        }
    }
    return 0;
}
auto internal::get_args() -> args_wrapper const& {
    return args;
}
