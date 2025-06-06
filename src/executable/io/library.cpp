#include "export.hpp"
#include "fs/export.hpp"
#include "util/lua.hpp"
#include "util/type_utility.hpp"
#include <iostream>
#include <filesystem>
using wow::io::writer_t;
using wow::io::reader_t;
using freader_t = wow::io::filereader_t;
using fwriter_t = wow::io::filewriter_t;
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
        check_open(L, path, Type<fwriter_t>::make(L, std::ofstream{path, std::ios::app}));
    } else {
        check_open(L, path, Type<fwriter_t>::make(L, std::ofstream{path}));
    }
    return 1;
}
static auto filewriter_create(lua_State* L) -> int {
    auto path = wow::fs::to_path(L, 1);
    auto append_mode = luaL_optboolean(L, 2, false);
    return push_open_filewriter(L, path, append_mode);
}
static auto filereader_create(lua_State* L) -> int {
    auto path = wow::fs::to_path(L, 1);
    check_open(L, path, Type<freader_t>::make(L, std::ifstream{path}));
    return 1;
}
void wow::io::library(lua_State* L, int idx) {
    lua::set_functions(L, idx, std::to_array<luaL_Reg>({
        {"filewriter", filewriter_create},
        {"filereader", filereader_create},
    }));
    Type<writer_t>::make(L, std::cout);
    lua_setfield(L, idx, "stdout");
    Type<writer_t>::make(L, std::cerr);
    lua_setfield(L, idx, "stderr");
    Type<reader_t>::make(L, std::cin);
    lua_setfield(L, idx, "stdin");
}
