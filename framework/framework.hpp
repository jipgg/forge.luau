#ifndef FRAMEWORK_HPP
#define FRAMEWORK_HPP

#include <memory>
#include <expected>
#include <filesystem>
#include <lualib.h>
#include <concepts>
#include "compile_time.hpp"
#include "common.hpp"

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
enum class Type {
    fs_path,
    COMPILE_TIME_ENUM_SENTINEL
};

namespace filesystem {
using path_t = std::filesystem::path;
constexpr auto path_tname = "filesystem_path";
void push_path(state_t L, const path_t& value);
void get(state_t L, const char* name = "filesystem");
}
namespace fileio {
void get(state_t L, const char* name = "fileio");
}
namespace json {
void get(state_t L, const char* name = "json");
}

namespace fw {
auto setup_state() -> state_owner_t;
auto load_script(state_t L, const std::filesystem::path& path) -> std::expected<state_t, std::string>;
constexpr auto as_string(state_t L, const std::string& str) -> int {
    lua_pushstring(L, str.c_str());
    return 1;
}
constexpr auto as_string(state_t L, const filesystem::path_t& p) -> int {
    lua_pushstring(L, p.string().c_str());
    return 1;
}
constexpr auto as_boolean(state_t L, bool b) -> int {
    lua_pushboolean(L, b);
    return 1;
}
constexpr auto as_path(state_t L, const filesystem::path_t& p) -> int {
    filesystem::push_path(L, p);
    return 1;
}
constexpr auto to_path(state_t L, int idx) -> filesystem::path_t& {
    return common::to_userdata_tagged<filesystem::path_t, Type::fs_path>(L, idx);
}
constexpr auto is_path(state_t L, int idx) -> bool {
    return common::has_userdata_tag<Type::fs_path>(L, idx);
}
constexpr auto check_path(state_t L, int idx) -> filesystem::path_t {
    if (is_path(L, idx)) return to_path(L, idx);
    else return luaL_checkstring(L, idx);
}
}
#endif
