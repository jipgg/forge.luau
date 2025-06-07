#include "export.hpp"
#include "lib/fs/export.hpp"
#include "lua/lua.hpp"
#include "lua/typeutility.hpp"
#include <iostream>
#include <filesystem>
using lib::io::writer;
using lib::io::reader;
using lib::io::filereader;
using lib::io::filewriter;
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
static auto check_open(lua_State* L, std::filesystem::path const& path, Ty& file) -> void {
    if (not file.is_open()) {
        luaL_errorL(L, "failed to open file '%s'.", path.string().c_str());
    }
}
static auto push_open_filewriter(lua_State* L, std::filesystem::path const& path, bool append) -> int {
    if (append) {
        check_open(L, path, lua::type<filewriter>::make(L, std::ofstream{path, std::ios::app}));
    } else {
        check_open(L, path, lua::type<filewriter>::make(L, std::ofstream{path}));
    }
    return 1;
}
static auto filewriter_create(lua_State* L) -> int {
    auto path = lib::fs::to_path(L, 1);
    auto append_mode = luaL_optboolean(L, 2, false);
    return push_open_filewriter(L, path, append_mode);
}
static auto filereader_create(lua_State* L) -> int {
    auto path = lib::fs::to_path(L, 1);
    check_open(L, path, lua::type<filereader>::make(L, std::ifstream{path}));
    return 1;
}
void lib::io::library(lua_State* L, int idx) {
    lua::set_functions(L, idx, std::to_array<luaL_Reg>({
        {"filewriter", filewriter_create},
        {"filereader", filereader_create},
    }));
    lua::type<writer>::make(L, std::cout);
    lua_setfield(L, idx, "stdout");
    lua::type<writer>::make(L, std::cerr);
    lua_setfield(L, idx, "stderr");
    lua::type<reader>::make(L, std::cin);
    lua_setfield(L, idx, "stdin");
}
