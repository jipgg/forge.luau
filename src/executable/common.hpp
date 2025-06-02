#pragma once
#include <expected>
#include <filesystem>
#include <lualib.h>
#include <Luau/Require.h>
#include "util/lua.hpp"
#include <fstream>
#include <expected>
#include <cassert>
#include <variant>
#include "util/type_utility.hpp"
#include "util/Interface.hpp"
#include <httplib.h>
using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using i8 = std::uint8_t;
using i16 = std::uint16_t;
using i32 = std::uint32_t;
using f32 = float;
using f64 = double;
auto result_create(lua_State* L) -> int;
auto push_directory_iterator(lua_State* L, std::filesystem::path const& directory, bool recursive) -> int;
auto to_path(lua_State* L, int idx) -> std::filesystem::path;
auto push_path(lua_State* L, std::filesystem::path const& path) -> int;
auto init_state(const char* libname = "lib") -> lua::StateOwner;
auto load_script(lua_State* L, const std::filesystem::path& path) -> std::expected<lua_State*, std::string>;
void open_require(lua_State* L);
//types
using Path = std::filesystem::path;
using Writer = Interface<std::ostream>;
using Reader = Interface<std::istream>;
using FileWriter = std::ofstream;
using FileReader = std::ifstream;
using HttpClient = httplib::Client;
//libraries
using Loader = void(*)(lua_State*L, int idx);
namespace loader {
void io(lua_State* L, int idx);
void filesystem(lua_State* L, int idx);
void process(lua_State* L, int idx);
void json(lua_State* L, int idx);
void http(lua_State* L, int idx);
}
//utility
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
static auto compile_options() -> T {
    static const char* userdata_types[] = {
        Type<Writer>::config.tname(),
        Type<FileWriter>::config.tname(),
        Type<Path>::config.tname(),
        Type<FileReader>::config.tname(),
        Type<Reader>::config.tname(),
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
