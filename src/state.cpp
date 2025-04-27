#include <Luau/Compiler.h>
#include <lualib.h>
#include <luacode.h>
#include <Luau/CodeGen.h>
#include <Luau/Require.h>
#include <fstream>
#include <filesystem>
#include <expected>
#include <iostream>
#include "common.hpp"
#include "utility/compile_time.hpp"
#include "Luau/ReplRequirer.h"
#include "Luau/Coverage.h"
#include <unordered_map>
#include <print>
#include <vector>
#include <functional>
#include "utility/luau.hpp"
#include "type_impl/method.hpp"
namespace fs = std::filesystem;
namespace rgs = std::ranges;
using cstr_t = const char*;
using std::string;

static auto loadstring(state_t L) -> int {
    auto l = size_t{};
    auto s = luaL_checklstring(L, 1, &l);
    auto chunkname = luaL_optstring(L, 2, s);
    lua_setsafeenv(L, LUA_ENVIRONINDEX, false);
    auto bytecode = luau::compile({s, l}, compile_options());

    return *luau::load(L, bytecode, {.chunkname = chunkname})
    .transform([] {
        return 1;
    }).transform_error([&](std::string err) {
        return luau::push_values(L, luau::nil, err);
    });
}
static int collectgarbage(lua_State* L) {
    std::string_view option = luaL_optstring(L, 1, "collect");
    if (option == "collect") {
        lua_gc(L, LUA_GCCOLLECT, 0);
        return luau::none;
    } else if (option == "count") {
        int count = lua_gc(L, LUA_GCCOUNT, 0);
        return luau::push(L, count);
    }
    luaL_error(L, "collectgarbage must be called with 'count' or 'collect'");
}
static auto useratom(const char* str, size_t len) -> int16_t {
    std::string_view namecall{str, len};
    static constexpr std::array info = compile_time::to_array<method>();
    auto found = rgs::find_if(info, [&namecall](auto const& e) {
        return e.name == namecall;
    });
    if (found == rgs::end(info)) return -1;
    return static_cast<int16_t>(found->value);
}

auto load_script(state_t L, fs::path const& path) -> std::expected<state_t, std::string> {
    auto main_thread = lua_mainthread(L);
    auto script_thread = lua_newthread(main_thread);
    std::ifstream file{path};
    if (!file.is_open()) {
        return std::unexpected(std::format("failed to open {}", path.string()));
    }
    std::string line, contents;
    while (std::getline(file, line)) contents.append(line + '\n');

    return luau::compile_and_load(script_thread, contents, compile_options(), {
        .chunkname = std::format("@{}", fs::absolute(path).replace_extension().generic_string())
    }).transform([&] {
        return script_thread;
    }).transform_error([](auto err) {
        return std::format("Loading error: {}", err);
    });
}
auto init_state(const char* libname) -> state_owner_t {
    auto globals = std::to_array<luaL_Reg>({
        {"loadstring", loadstring},
        {"collectgarbage", collectgarbage},
    });
    auto state = luau::new_state({
        .useratom = useratom,
        .globals = globals,
    });
    auto L = state.get();
    open_require(L);
    path::init(L);
    writer::init(L);
    filewriter::init(L);
    lua_newtable(L);
    lib::io::as_field(L);
    lib::garbage::as_field(L);
    lib::filesystem::as_field(L);
    lib::process::as_field(L);
    lua_setglobal(L, libname);
    luaL_sandbox(L);
    return state;
}
