#ifndef FRAMEWORK_HPP
#define FRAMEWORK_HPP

#include <memory>
#include <expected>
#include <filesystem>
#include <lualib.h>
#include "compile_time.hpp"
using state_t = lua_State*;
using state_owner_t = std::unique_ptr<lua_State, decltype(&lua_close)>;

struct Lib_config {
    const char* name;
    bool global;
    void apply(state_t L, const luaL_Reg* lib) {
        if (global and name) {
            luaL_register(L, name, lib);
            return;
        } else if (global) {
            lua_pushvalue(L, LUA_GLOBALSINDEX);
            luaL_register(L, nullptr, lib);
            lua_pop(L, 1);
            return;
        }
        lua_newtable(L);
        luaL_register(L, nullptr, lib);
        if (name) lua_setfield(L, -2, name);
    }
};
void open_filesystem(state_t L, Lib_config config = {.name = "filesystem"});
void open_fileio(state_t L, Lib_config config = {.name = "fileio"});
void open_json(state_t L, Lib_config config = {.name = "json"});

auto setup_state() -> state_owner_t;
auto load_script(state_t L, const std::filesystem::path& path) -> std::expected<state_t, std::string>;

using path_t = std::filesystem::path;
void register_path(state_t L);
auto push_path(state_t L, const path_t& path) -> int;
auto to_path(state_t L, int idx) -> path_t;

enum class Method {
    read_all,
    children,
    string,
    extension,
    has_extension,
    replace_extension,
    has_stem,
    stem,
    filename,
    has_filename,
    replace_filename,
    remove_filename,
    parent_path,
    has_parent_path,
    relative_path,
    has_relative_path,
    is_absolute,
    is_relative,
    root_directory,
    has_root_directory,
    lexically_normal,
    lexically_proximate,
    lexically_relative,
    COMPILE_TIME_ENUM_SENTINEL
};

#endif
