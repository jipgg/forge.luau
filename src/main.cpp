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

static bool codegen = false;

static auto copts() -> Luau::CompileOptions {
    Luau::CompileOptions result = {};
    result.optimizationLevel = 2;
    result.debugLevel = 3;
    result.typeInfoLevel = 1;
    result.coverageLevel = 2;
    const char* userdata_types[] ={
        path_name(),
        nullptr
    };
    result.userdataTypes = userdata_types;
    return result;
}

static auto loadstring(state_t L) -> int {
    size_t l = 0;
    const char* s = luaL_checklstring(L, 1, &l);
    const char* chunkname = luaL_optstring(L, 2, s);
    lua_setsafeenv(L, LUA_ENVIRONINDEX, false);
    std::string bytecode = Luau::compile(std::string(s, l), copts());

    return *luau::load(L, bytecode, chunkname)
    .transform([] {
        return 1;
    }).transform_error([&](std::string err) {
        return luau::push_values(L, luau::nil, err);
    });
}


// i should make my own require resolver mechanism eventually
// this code is crazy
struct require_context: public RequireResolver::RequireContext {
    require_context(std::string source): source(std::move(source)) {}
    auto getPath() -> std::string override {return source.substr(1);}
    auto isRequireAllowed() -> bool override {
        return isStdin() || (!source.empty() && source[0] == '@');
    }
    auto isStdin() -> bool override {
        return source == "=stdin";
    }
    auto createNewIdentifer(const std::string& path) -> std::string override {
        return "@" + path;
    }
    std::string source;
};
struct cache_manager: public RequireResolver::CacheManager {
    cache_manager(state_t state): state(state) {}
    auto isCached(const std::string& path) -> bool override {
        luaL_findtable(state, LUA_REGISTRYINDEX, "_MODULES", 1);
        lua_getfield(state, -1, path.c_str());
        bool cached = !lua_isnil(state, -1);
        lua_pop(state, 2);
        if (cached) cache_key = path;
        return cached;
    }
    std::string cache_key;
    state_t state;
};
constexpr auto cached = RequireResolver::ModuleStatus::Cached;
struct error_handler: RequireResolver::ErrorHandler {
    error_handler(state_t state): state() {}
    void reportError(const std::string message) override {
        luaL_errorL(state, "%s", message.c_str());
    }
    state_t state;
};
static auto resolve_require(state_t L) -> decltype(auto) {
    std::string name = luaL_checkstring(L, 1);
    lua_Debug ar;
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

static auto require(lua_State* L) -> int {
    auto finish = [&] {
        if (lua_isstring(L, -1)) lua_error(L);
        return 1;
    };
    auto resolved = resolve_require(L);

    if (resolved.status == cached) return finish();

    // module needs to run in a new thread, isolated from the rest
    // note: we create ML on main thread so that it doesn't inherit environment of L
    lua_State* GL = lua_mainthread(L);
    lua_State* ML = lua_newthread(GL);
    lua_xmove(GL, L, 1);

    // new thread needs to have the globals sandboxed
    luaL_sandboxthread(ML);

    // now we can compile & run module on the new thread
    std::string bytecode = Luau::compile(resolved.sourceCode, copts());
    if (luau_load(ML, resolved.identifier.c_str(), bytecode.data(), bytecode.size(), 0) == 0) {
        if (codegen) {
            Luau::CodeGen::CompilationOptions nativeOptions;
            Luau::CodeGen::compile(ML, -1, nativeOptions);
        }

        int status = lua_resume(ML, L, 0);
        if (status == 0) {
            if (lua_gettop(ML) == 0)
                lua_pushstring(ML, "module must return a value");
            else if (!lua_istable(ML, -1) && !lua_isfunction(ML, -1))
                lua_pushstring(ML, "module must return a table or function");
        }
        else if (status == LUA_YIELD) {
            lua_pushstring(ML, "module can not yield");
        }
        else if (!lua_isstring(ML, -1)) {
            lua_pushstring(ML, "unknown error while running module");
        }
    }

    // there's now a return value on top of ML; L stack: _MODULES ML
    lua_xmove(ML, L, 1);
    lua_pushvalue(L, -1);
    lua_setfield(L, -4, resolved.absolutePath.c_str());

    // L stack: _MODULES ML result
    return finish();
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
#ifdef CALLGRIND
static int lua_callgrind(lua_State* L)
{
    const char* option = luaL_checkstring(L, 1);

    if (strcmp(option, "running") == 0)
    {
        int r = RUNNING_ON_VALGRIND;
        lua_pushboolean(L, r);
        return 1;
    }

    if (strcmp(option, "zero") == 0)
    {
        CALLGRIND_ZERO_STATS;
        return 0;
    }

    if (strcmp(option, "dump") == 0)
    {
        const char* name = luaL_checkstring(L, 2);

        CALLGRIND_DUMP_STATS_AT(name);
        return 0;
    }

    luaL_error(L, "callgrind must be called with one of 'running', 'zero', 'dump'");
}
#endif

auto load_script(state_t L, const fs::path& path) -> std::expected<state_t, std::string> {
    auto main_thread = lua_mainthread(L);
    auto script_thread = lua_newthread(main_thread);
    std::ifstream file{path};
    if (!file.is_open()) {
        return std::unexpected(std::format("failed to open {}", path.string()));
    }
    std::string line, contents;
    while (std::getline(file, line)) contents.append(line + '\n');
    auto bytecode = Luau::compile(contents, copts());
    auto chunkname = std::format("@{}", normalizePath(path.string()));
    auto status = luau_load(script_thread, chunkname.c_str(), bytecode.data(), bytecode.size(), 0);
    if (status != LUA_OK) {
        std::string error_message{lua_tostring(script_thread, -1)};
        lua_pop(script_thread, 1);
        return std::unexpected{error_message};
    }
    return script_thread;
}
static auto user_atom(const char* str, size_t len) -> int16_t {
    std::string_view namecall{str, len};
    constexpr std::array info = compile_time::to_array<method_name>();
    auto found = rgs::find_if(info, [&namecall](decltype(info[0])& e) {
        return e.name == namecall;
    });
    if (found == rgs::end(info)) return -1;
    return static_cast<int16_t>(found->value);
}

auto setup_state() -> state_owner_t {
    auto L = luaL_newstate();
    lua_callbacks(L)->useratom = user_atom;
    if (codegen) Luau::CodeGen::create(L);
    luaL_openlibs(L);
    static const luaL_Reg funcs[] = {
        {"loadstring", loadstring},
        {"require", require},
        {"collectgarbage", collectgarbage},
#ifdef CALLGRIND
        {"callgrind", lua_callgrind},
#endif
        {NULL, NULL},
    };
    lua_pushvalue(L, LUA_GLOBALSINDEX);
    luaL_register(L, nullptr, funcs);
    lua_pop(L, 1);
    //luaL_sandbox(L);
    return {L, lua_close};
}

auto main() -> int {
    auto state = setup_state();
    auto L = state.get();
    register_path(L);
    open_filesystem(L, {.name = "filesystem"});
    open_json(L, {.name = "json"});
    open_fileio(L, {.name = "fileio"});
    luaL_sandbox(L);

    auto expected = load_script(state.get(), "abc.luau");
    if (!expected) {
        std::println("\033[35mError: {}\033[0m", expected.error());
        return -1;
    }

    auto status = lua_resume(*expected, state.get(), 0);
    if (status != LUA_OK) {
        std::println("\033[35mError: {}\033[0m", luaL_checkstring(*expected, -1));
    }
    return 0;
}
