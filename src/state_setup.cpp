#include <Luau/Compiler.h>
#include <lualib.h>
#include <Luau/CodeGen.h>
#include <Luau/Require.h>
#include <Luau/FileUtils.h>
#include <fstream>
#include <filesystem>
#include <expected>
#include "common.hpp"
#include "utility/compile_time.hpp"
#include "utility/luau.hpp"
#include <print>
namespace fs = std::filesystem;
namespace rgs = std::ranges;
using cstr_t = const char*;
using std::string;

static bool codegen = true;

static auto compile_options() -> lua_CompileOptions {
    static cstr_t userdata_types[] = {
        path_builder_t::name,
        filewriter_builder_t::name,
        nullptr
    };
    return {
        .optimizationLevel = 2,
        .debugLevel = 3,
        .typeInfoLevel = 2,
        .coverageLevel = 2,
        .userdataTypes = userdata_types,
    };
}
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


// i should make my own require resolver mechanism eventually
// this code is crazy
struct require_context: public RequireResolver::RequireContext {
    require_context(string const& source): source(source) {}
    auto getPath() -> string override {return source.substr(1);}
    auto isRequireAllowed() -> bool override {
        return isStdin() || (!source.empty() && source[0] == '@');
    }
    auto isStdin() -> bool override {
        return source == "=stdin";
    }
    auto createNewIdentifer(string const& path) -> string override {
        return "@" + path;
    }
    string source;
};
struct cache_manager: public RequireResolver::CacheManager {
    cache_manager(state_t state): state(state) {}
    auto isCached(string const& path) -> bool override {
        luaL_findtable(state, LUA_REGISTRYINDEX, "_MODULES", 1);
        lua_getfield(state, -1, path.c_str());
        bool cached = !lua_isnil(state, -1);
        lua_pop(state, 2);
        if (cached) cache_key = path;
        return cached;
    }
    string cache_key;
    state_t state;
};
constexpr auto cached = RequireResolver::ModuleStatus::Cached;
struct error_handler: RequireResolver::ErrorHandler {
    error_handler(state_t state): state() {}
    void reportError(string const message) override {
        luaL_errorL(state, "%s", message.c_str());
    }
    state_t state;
};
static auto resolve_require(state_t L) -> decltype(auto) {
    auto name = string(luaL_checkstring(L, 1));
    auto ar = lua_Debug{};
    lua_getinfo(L, 1, "s", &ar);
    auto rc = require_context{ar.source};
    auto cm = cache_manager{L};
    auto eh = error_handler{L};
    auto resolver = RequireResolver{name, rc, cm, eh};
    return resolver.resolveRequire([&](auto status) {
        lua_getfield(L, LUA_REGISTRYINDEX, "_MODULES");
        if (status == cached) {
            lua_getfield(L, -1, cm.cache_key.c_str());
        }
    });
}

static auto require(state_t L) -> int {
    auto finish = [&] {
        if (lua_isstring(L, -1)) lua_error(L);
        return 1;
    };
    auto resolved = resolve_require(L);
    if (resolved.status == cached) return 1;

    // module needs to run in a new thread, isolated from the rest
    // note: we create ML on main thread so that it doesn't inherit environment of L
    lua_State* GL = lua_mainthread(L);
    lua_State* ML = lua_newthread(GL);
    lua_xmove(GL, L, 1);

    // new thread needs to have the globals sandboxed
    luaL_sandboxthread(ML);

    // now we can compile & run module on the new thread
    //std::string bytecode = Luau::compile(resolved.sourceCode, copts());
    auto loaded_correctly = luau::compile_and_load(ML, resolved.sourceCode, compile_options(), {
        .chunkname = resolved.identifier
    });
    if (not loaded_correctly) {
        luau::push(L, loaded_correctly.error());
        lua_error(L);
    }
    switch (lua_resume(ML, L, 0)) {
        case LUA_OK:
            if (lua_gettop(ML) == 0) {
                luaL_errorL(L, "module must return a value");
            }
            break;
        case LUA_YIELD:
            luaL_errorL(L, "module can not yield");
            break;
    }
    lua_xmove(ML, L, 1);
    lua_pushvalue(L, -1);
    lua_setfield(L, -4, resolved.absolutePath.c_str());
    return 1;
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
        .chunkname = std::format("@{}", normalizePath(path.string()))
    }).transform([&] {
        return script_thread;
    }).transform_error([](auto err) {
        return std::format("Loading error: {}", err);
    });
}
static auto user_atom(const char* str, size_t len) -> int16_t {
    std::string_view namecall{str, len};
    static constexpr std::array info = compile_time::to_array<method_name>();
    auto found = rgs::find_if(info, [&namecall](auto const& e) {
        return e.name == namecall;
    });
    if (found == rgs::end(info)) return -1;
    return static_cast<int16_t>(found->value);
}

auto setup_state() -> state_owner_t {
    auto globals = std::to_array<luaL_Reg>({
        {"loadstring", loadstring},
        {"require", require},
        {"collectgarbage", collectgarbage},
    });
    auto state = luau::new_state({
        .useratom = user_atom,
        .globals = globals,
    });
    auto L = state.get();
    register_path(L);
    register_filewriter(L);
    lua_newtable(L);
    open_filesystem(L, {.name = "filesystem", .local = true});
    open_json(L, {.name = "json", .local = true});
    open_fileio(L, {.name = "fileio", .local = true});
    open_process(L, {.name = "process", .local = true});
    lua_setglobal(L, "builtin");
    luaL_sandbox(L);
    return state;
}
