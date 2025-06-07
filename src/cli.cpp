#include "export.hpp"
#include <print>
#include <ranges>
#include <filesystem>
namespace vws = std::views;
namespace fs = std::filesystem;
using namespace std::string_view_literals;

struct args_wrapper {
    int argc;
    char** argv;
    auto span() const -> std::span<char*> {
        return {argv, argv + argc};
    }
    auto view() const -> decltype(auto) {
        return std::views::transform(span(), [](auto v) -> std::string_view {return v;});
    }
    auto operator[](size_t idx) const -> std::optional<std::string_view> {
        if (idx >= argc) return std::nullopt;
        return argv[idx];
    }
    args_wrapper(int argc, char** argv): argc(argc), argv(argv) {}
    explicit args_wrapper() = default;
};

static auto run_main_entry_script(args_wrapper const& args, lua::state L, std::string_view script) -> void {
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
template <typename T>
constexpr auto as() {
    return vws::transform([](auto&& v) -> T {
        return static_cast<T>(std::forward<decltype(v)>(v));
    });
}

auto main(int argc, char** argv) -> int {
    auto args = args_wrapper{argc, argv};
    auto state = init_state("pls");
    auto L = state.get();
    auto scripts = vws::filter([](std::string_view e) {
        return e.ends_with(".luau");
    });
    auto args_sv = args.view() | as<std::string_view>();
    for (auto script : args.view() | scripts) {
        constexpr auto wildcard = "*.luau"sv;
        if (script.ends_with(wildcard)) {
            auto dir = fs::path(script).parent_path();
            for (auto entry : fs::directory_iterator(dir)) {
                auto const path = entry.path();
                if (not entry.is_regular_file() or path.extension() != ".luau") {
                    continue;
                }
                run_main_entry_script(args, L, path.string());
            }
        } else {
            run_main_entry_script(args, L, script);
        }
    }
    //std::system("pause");
    return 0;
}
