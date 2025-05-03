#pragma once
#include <expected>
#include <filesystem>
#include <lualib.h>
#include <Luau/Require.h>
#include "util/lua.hpp"
#include <fstream>
#include <expected>
#include <cassert>
#include "type_defs.hpp"
auto result_create(lua_State* L) -> int;
auto push_directory_iterator(lua_State* L, path const& directory, bool recursive) -> int;
auto to_path(lua_State* L, int idx) -> path;
auto push_path(lua_State* L, path const& path) -> int;
auto init_state(const char* libname = "lib") -> lua::state_ptr;
auto load_script(lua_State* L, const std::filesystem::path& path) -> std::expected<lua_State*, std::string>;
void open_require(lua_State* L);

#define declare_library(lib, name)\
constexpr const char* lib##_name = name;\
void push_##lib(lua_State* L);\
inline void open_##lib(lua_State* L) {\
    push_##lib(L);\
    lua_setglobal(L, lib##_name);\
}\
inline void setfield_##lib(lua_State* L, int idx) {\
    push_##lib(L);\
    lua_setfield(L, idx, lib##_name);\
}\
inline void register_##lib(lua_State* L) {\
    push_##lib(L);\
    lua_setfield(L, -2, lib##_name);\
}
#define declare_type(type)\
constexpr const char* type##_name = #type;\
void load_##type(lua_State* L);\
auto new_##type(lua_State* L, type&& value) -> type&;\
auto check_##type(lua_State* L, int idx) -> type&;\
auto as_##type(lua_State* L, int idx) -> type&;\
auto test_namecall_##type(lua_State* L, type& self, int atom) -> std::optional<int>
declare_type(path);
declare_type(writer);
declare_type(result);
declare_type(filewriter);
declare_type(reader);
declare_library(filesystem, "fs");
declare_library(io, "io");
declare_library(process, "proc");
declare_library(garbage, "gc");
declare_library(monad, "monad");
#undef declare_library
#undef declare_type

inline void push_library(lua_State* L, std::span<const luaL_Reg> api) {
    lua_newtable(L);
    for (auto const& entry : api) {
        lua_pushcfunction(L, entry.func, entry.name);
        lua_setfield(L, -2, entry.name);
    }
}
enum class read_file_error {
    not_a_file,
    failed_to_open,
    failed_to_read,
};;
inline auto read_file(std::filesystem::path const& path) -> std::expected<std::string, read_file_error> {
    using err = read_file_error;
    if (not std::filesystem::is_regular_file(path)) {
        return std::unexpected(err::not_a_file);
    }
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    if (!file) return std::unexpected(err::failed_to_open);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);
    auto buf = std::string(size, '\0');
    if (not file.read(&buf.data()[0], size)) {
        return std::unexpected(err::failed_to_read);
    }
    return buf;
}
template <class type>
concept compile_options_ish = requires (type v) {
    v.optimizationLevel;
    v.debugLevel;
    v.typeInfoLevel;
    v.coverageLevel;
    v.userdataTypes;
};

template <class type = lua_CompileOptions>
requires compile_options_ish<type>
static auto compile_options() -> type {
    static const char* userdata_types[] = {
        path_name,
        writer_name,
        result_name,
        filewriter_name,
        nullptr
    };
    auto opts = type{};
    opts.optimizationLevel = 2;
    opts.debugLevel = 3;
    opts.typeInfoLevel = 2;
    opts.coverageLevel = 2;
    opts.userdataTypes = userdata_types;
    return opts;
}
namespace internal {
    auto get_args() -> args_wrapper const&;
}
