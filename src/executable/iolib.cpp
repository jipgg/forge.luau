#include "common.hpp"
#include <iostream>
#include <filesystem>
namespace fs = std::filesystem;
static auto scan(lua_State* L) -> int {
    auto str = std::string{};
    std::cin >> str;
    return lua::push(L, str);
}
static auto write(lua_State* L) -> int {
    std::cout << luaL_checkstring(L, 1);
    return lua::none;
}
template<class Ty>
concept has_is_open = requires (Ty& v) {
{v.is_open()} -> std::same_as<bool>;
};

template<has_is_open Ty>
static auto check_open(lua_State* L, fs::path const& path, Ty& file) -> void {
    if (not file.is_open()) {
        luaL_errorL(L, "failed to open file '%s'.", path.string().c_str());
    }
}
static auto push_open_filewriter(lua_State* L, fs::path const& path, bool append) -> int {
    if (append) {
        check_open(L, path, Type<FileWriter>::make(L, std::ofstream{path, std::ios::app}));
    } else {
        check_open(L, path, Type<FileWriter>::make(L, std::ofstream{path}));
    }
    return 1;
}
static auto filewriter_create(lua_State* L) -> int {
    auto path = to_path(L, 1);
    auto append_mode = luaL_optboolean(L, 2, false);
    return push_open_filewriter(L, path, append_mode);
}

void open_iolib(lua_State* L) {
    constexpr auto lib = std::to_array<luaL_Reg>({
        {"filewriter", filewriter_create},
    });
    push_library(L, lib);
    Type<Writer>::make(L, std::cout);
    lua_setfield(L, -2, "stdout");
    Type<Writer>::make(L, std::cerr);
    lua_setfield(L, -2, "stderr");
    Type<Reader>::make(L, std::cin);
    lua_setfield(L, -2, "stdin");
    lua_setglobal(L, "io");
}
