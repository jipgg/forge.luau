#pragma once
#include <expected>
#include <filesystem>
#include <lualib.h>
#include <Luau/Require.h>
#include "lua/lua.hpp"
#include <expected>
#include <cassert>
#include "lua/typeutility.hpp"
#include <lib/json/export.hpp>
#include <lib/io/export.hpp>
#include <lib/proc/export.hpp>
#include <lib/fs/export.hpp>
#include <lib/http/export.hpp>
#include <httplib.h>
auto init_state(const char* libname = "lib") -> lua::state_owner;
auto load_script(lua_State* L, const std::filesystem::path& path) -> std::expected<lua_State*, std::string>;
void open_require(lua_State* L);

using loader = void(*)(lua_State*L, int idx);
template <loader F>
constexpr void push(lua_State* L) {
    lua_newtable(L);
    F(L, -2);
}
template <loader F>
constexpr void setfield(lua_State* L, int idx, const char* name) {
    lua_newtable(L);
    F(L, -2);
    lua_setfield(L, idx, name);
}
template <loader F>
constexpr void open(lua_State* L, const char* name) {
    lua_newtable(L);
    F(L, -2);
    lua_setglobal(L, name);
}

template <lua::compile_options_ish T = lua_CompileOptions>
constexpr auto compile_options() -> T {
    using lua::type;
    static const char* userdata_types[] = {
        type<lib::fs::path>::config.tname(),
        type<lib::http::client>::config.tname(),
        type<lib::http::response>::config.tname(),
        type<lib::io::filewriter>::config.tname(),
        type<lib::io::filereader>::config.tname(),
        type<lib::io::writer>::config.tname(),
        type<lib::io::reader>::config.tname(),
        nullptr
    };
    auto opts = T{};
    opts.optimizationLevel = 2;
    opts.debugLevel = 3;
    opts.typeInfoLevel = 2;
    opts.coverageLevel = 2;
    //opts.userdataTypes = userdata_types;
    return opts;
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
