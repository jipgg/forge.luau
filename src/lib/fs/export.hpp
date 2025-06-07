#pragma once
#include <filesystem>
struct lua_State;

namespace lib::fs {
void library(lua_State* L, int idx);
using path = std::filesystem::path;
auto to_path(lua_State* L, int idx) -> path;
auto push_directory_iterator(lua_State* L, const path& path, bool recursive) -> int;
auto push_path(lua_State* L, const path& path) -> int;
}
