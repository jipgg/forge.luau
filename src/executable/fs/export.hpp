#pragma once
#include <filesystem>
struct lua_State;

namespace wow::fs {
void library(lua_State* L, int idx);
using path_t = std::filesystem::path;
auto to_path(lua_State* L, int idx) -> path_t;
auto push_directory_iterator(lua_State* L, const path_t& path, bool recursive) -> int;
auto push_path(lua_State* L, const path_t& path) -> int;
}
