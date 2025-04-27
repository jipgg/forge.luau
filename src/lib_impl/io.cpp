#include "common.hpp"
#include <iostream>
#include <filesystem>
namespace fs = std::filesystem;
static auto read_line(state_t L) -> int {
    auto str = std::string{};
    std::cin >> str;
    luau::make_buffer(L, str);
    return 1;
}
static auto read(state_t L) -> int {
    if (lua_isnone(L, 1)) {
        auto str = std::string{};
        std::cin >> str;
        luau::make_buffer(L, str);
        return 1;
    }
    if (lua_isbuffer(L, 1)) {
        auto buf = luau::to_buffer(L, 1);
        std::cin.read(&buf.front(), buf.size());
        return luau::none;
    }
    auto const count = luaL_checkinteger(L, 1);
    auto buf = luau::make_buffer(L, count);
    std::cin.read(&buf.front(), buf.size());
    return 1;
}
static auto scan(state_t L) -> int {
    auto str = std::string{};
    std::cin >> str;
    return luau::push(L, str);
}
static auto write(state_t L) -> int {
    std::cout << luaL_checkstring(L, 1);
    return luau::none;
}
static auto err_write(state_t L) -> int {
    std::cerr << luaL_checkstring(L, 1);
    return luau::none;
}
template<class Ty>
concept has_is_open = requires (Ty& v) {
{v.is_open()} -> std::same_as<bool>;
};

template<has_is_open Ty>
static auto check_open(state_t L, fs::path const& path, Ty& file) -> void {
    if (not file.is_open()) {
        luaL_errorL(L, "failed to open file '%s'.", path.string().c_str());
    }
}
static auto file_writer(state_t L) -> int {
    auto path = to_path(L, 1);
    auto append_mode = luaL_optboolean(L, 2, false);
    auto file = std::ofstream{};
    if (append_mode) {
        check_open(L, path, filewriter::util::make(L, path, std::ios::app));
    } else {
        check_open(L, path, filewriter::util::make(L, path));
    }
    return 1;
}

template <>
auto lib::io::name() -> std::string {return "io";};
template<>
void lib::io::push(state_t L) {
    constexpr auto lib = std::to_array<luaL_Reg>({
        {"scan", scan},
        {"file_writer", file_writer},
    });
    push_api(L, lib);
    writer::util::make(L, &std::cout);
    lua_setfield(L, -2, "stdout");
    writer::util::make(L, &std::cerr);
    lua_setfield(L, -2, "stderr");
}
