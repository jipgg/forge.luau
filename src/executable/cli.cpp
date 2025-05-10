#include "common.hpp"
#include <print>
#include <ranges>
#include <filesystem>
namespace vws = std::views;
namespace fs = std::filesystem;
using namespace std::string_view_literals;

static ArgsWrapper args;

static auto run_script(lua_State* L, std::string_view script) -> void {
	std::println("{}", fs::current_path().string());
    auto state = load_script(L, script);
    if (!state) {
        std::println("\033[35mError: {}\033[0m", state.error());
        return;
    }
    for (auto arg : args.span()) lua_pushstring(*state, arg);
    auto status = lua_resume(*state, L, args.argc);

    if (status != LUA_OK) {
        std::println("\033[35mError: {}\033[0m", luaL_checkstring(*state, -1));
    }
}

auto main(int argc, char** argv) -> int {
    args = ArgsWrapper{argc, argv};
    auto state = init_state("pls");
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
auto internal::get_args() -> ArgsWrapper const& {
    return args;
}
