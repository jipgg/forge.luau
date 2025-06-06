#pragma once
#include <expected>
#include <filesystem>
#include <lualib.h>
#include <Luau/Require.h>
#include "util/lua.hpp"
#include <expected>
#include <cassert>
#include "util/type_utility.hpp"
#include <json/export.hpp>
#include <io/export.hpp>
#include <proc/export.hpp>
#include <fs/export.hpp>
#include <http/export.hpp>
#include <httplib.h>
using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using i8 = std::uint8_t;
using i16 = std::uint16_t;
using i32 = std::uint32_t;
using f32 = float;
using f64 = double;
auto init_state(const char* libname = "lib") -> lua::StateOwner;
auto load_script(lua_State* L, const std::filesystem::path& path) -> std::expected<lua_State*, std::string>;
void open_require(lua_State* L);
using Loader = void(*)(lua_State*L, int idx);
template <Loader F>
constexpr void push(lua_State* L) {
    lua_newtable(L);
    F(L, -2);
}
template <Loader F>
constexpr void setfield(lua_State* L, int idx, const char* name) {
    lua_newtable(L);
    F(L, -2);
    lua_setfield(L, idx, name);
}
template <Loader F>
constexpr void open(lua_State* L, const char* name) {
    lua_newtable(L);
    F(L, -2);
    lua_setglobal(L, name);
}

template <lua::CompileOptionsIsh T = lua_CompileOptions>
constexpr auto compile_options() -> T {
    static const char* userdata_types[] = {
        Type<wow::fs::path_t>::config.tname(),
        Type<wow::http::client_t>::config.tname(),
        Type<wow::http::response_t>::config.tname(),
        Type<wow::io::filewriter_t>::config.tname(),
        Type<wow::io::filereader_t>::config.tname(),
        Type<wow::io::writer_t>::config.tname(),
        Type<wow::io::reader_t>::config.tname(),
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
