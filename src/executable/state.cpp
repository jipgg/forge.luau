#include <Luau/Compiler.h>
#include <lualib.h>
#include <luacode.h>
#include <Luau/CodeGen.h>
#include <Luau/Require.h>
#include <fstream>
#include <filesystem>
#include <expected>
#include "common.hpp"
#include "util/comptime.hpp"
#include "NamedAtom.hpp"
namespace fs = std::filesystem;
namespace sr = std::ranges;
using std::string;

static auto loadstring(lua_State* L) -> int {
    auto l = size_t{};
    auto s = luaL_checklstring(L, 1, &l);
    auto chunkname = luaL_optstring(L, 2, s);
    lua_setsafeenv(L, LUA_ENVIRONINDEX, false);
    auto bytecode = lua::compile({s, l}, compile_options());

    return *lua::load(L, bytecode, {.chunkname = chunkname})
    .transform([] {
        return 1;
    }).transform_error([&](std::string err) {
        return lua::push_tuple(L, lua::nil, err);
    });
}
static int collectgarbage(auto L) {
    std::string_view option = luaL_optstring(L, 1, "collect");
    if (option == "collect") {
        lua_gc(L, LUA_GCCOLLECT, 0);
        return lua::none;
    } else if (option == "count") {
        int count = lua_gc(L, LUA_GCCOUNT, 0);
        return lua::push(L, count);
    }
    luaL_error(L, "collectgarbage must be called with 'count' or 'collect'");
}
static auto useratom(const char* str, size_t len) -> int16_t {
    std::string_view namecall{str, len};
    static constexpr auto info = comptime::to_array<NamedAtom>();
    auto found = sr::find_if(info, [&namecall](auto const& e) {
        return e.name == namecall;
    });
    if (found == sr::end(info)) return -1;
    return static_cast<int16_t>(found->value);
}

auto load_script(lua_State* L, fs::path const& path) -> std::expected<lua_State*, std::string> {
    auto main_thread = lua_mainthread(L);
    auto script_thread = lua_newthread(main_thread);
    luaL_sandboxthread(script_thread);
    std::ifstream file{path};
    if (!file.is_open()) {
        return std::unexpected(std::format("failed to open {}", path.string()));
    }
    std::string line, contents;
    while (std::getline(file, line)) contents.append(line + '\n');

    return lua::compile_and_load(script_thread, contents, compile_options(), {
        .chunkname = std::format("@{}", fs::absolute(path).replace_extension().generic_string())
    }).transform([&] {
        return script_thread;
    }).transform_error([](auto err) {
        return std::format("Loading error: {}", err);
    });
}
auto init_state(const char* libname) -> lua::StateOwner {
    auto globals = std::to_array<luaL_Reg>({
        {"loadstring", loadstring},
        {"collectgarbage", collectgarbage},
    });
    auto state = lua::new_state({
        .useratom = useratom,
        .globals = globals,
    });
    auto L = state.get();
    open_require(L);
    Type<Path>::setup(L);
    Type<Writer>::setup(L);
    Type<FileWriter>::setup(L);
    Type<HttpClient>::setup(L);
    open_fslib(L);
    open_iolib(L);
    open_oslib(L);
    open_httplib(L);
    open_jsonlib(L);
    luaL_sandbox(L);
    return state;
}
