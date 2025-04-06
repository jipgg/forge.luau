// most of this source code is yanked from the Repl source in luau
#include <Luau/Compiler.h>
#include <lualib.h>
#include <Luau/CodeGen.h>
#include <Luau/Require.h>
#include <Luau/FileUtils.h>
#include <fstream>
#include <filesystem>
#include <expected>
#include "framework.hpp"
#include "compile_time.hpp"
namespace fs = std::filesystem;
namespace rgs = std::ranges;

static bool codegen = false;

auto copts() -> Luau::CompileOptions {
    Luau::CompileOptions result = {};
    result.optimizationLevel = 2;
    result.debugLevel = 3;
    result.typeInfoLevel = 1;
    result.coverageLevel = 2;
    return result;
}

static auto lua_loadstring(lua_State* L) -> int {
    size_t l = 0;
    const char* s = luaL_checklstring(L, 1, &l);
    const char* chunkname = luaL_optstring(L, 2, s);

    lua_setsafeenv(L, LUA_ENVIRONINDEX, false);

    std::string bytecode = Luau::compile(std::string(s, l), copts());
    if (luau_load(L, chunkname, bytecode.data(), bytecode.size(), 0) == 0)
        return 1;

    lua_pushnil(L);
    lua_insert(L, -2); // put before error message
    return 2;          // return nil plus error message
}

static auto finishrequire(lua_State* L) -> int {
    if (lua_isstring(L, -1))
        lua_error(L);
    return 1;
}

struct RuntimeRequireContext : public RequireResolver::RequireContext {
    explicit RuntimeRequireContext(std::string source): source(std::move(source)) {}

    std::string getPath() override {
        return source.substr(1);
    }
    bool isRequireAllowed() override {
        return isStdin() || (!source.empty() && source[0] == '@');
    }

    bool isStdin() override {
        return source == "=stdin";
    }
    std::string createNewIdentifer(const std::string& path) override {
        return "@" + path;
    }

private:
    std::string source;
};

struct RuntimeCacheManager : public RequireResolver::CacheManager
{
    explicit RuntimeCacheManager(lua_State* L): L(L) {}

    bool isCached(const std::string& path) override {
        luaL_findtable(L, LUA_REGISTRYINDEX, "_MODULES", 1);
        lua_getfield(L, -1, path.c_str());
        bool cached = !lua_isnil(L, -1);
        lua_pop(L, 2);

        if (cached)
            cacheKey = path;

        return cached;
    }

    std::string cacheKey;

private:
    lua_State* L;
};

struct RuntimeErrorHandler : RequireResolver::ErrorHandler
{
    explicit RuntimeErrorHandler(lua_State* L)
        : L(L)
    {
    }

    void reportError(const std::string message) override
    {
        luaL_errorL(L, "%s", message.c_str());
    }

private:
    lua_State* L;
};

static int lua_require(lua_State* L) {
    std::string name = luaL_checkstring(L, 1);

    RequireResolver::ResolvedRequire resolvedRequire;
    {
        lua_Debug ar;
        lua_getinfo(L, 1, "s", &ar);

        RuntimeRequireContext requireContext{ar.source};
        RuntimeCacheManager cacheManager{L};
        RuntimeErrorHandler errorHandler{L};

        RequireResolver resolver(std::move(name), requireContext, cacheManager, errorHandler);

        resolvedRequire = resolver.resolveRequire(
            [L, &cacheKey = cacheManager.cacheKey](const RequireResolver::ModuleStatus status)
            {
lua_getfield(L, LUA_REGISTRYINDEX, "_MODULES");
                if (status == RequireResolver::ModuleStatus::Cached)
                    lua_getfield(L, -1, cacheKey.c_str());
            }
        );
    }

    if (resolvedRequire.status == RequireResolver::ModuleStatus::Cached)
        return finishrequire(L);

    // module needs to run in a new thread, isolated from the rest
    // note: we create ML on main thread so that it doesn't inherit environment of L
    lua_State* GL = lua_mainthread(L);
    lua_State* ML = lua_newthread(GL);
    lua_xmove(GL, L, 1);

    // new thread needs to have the globals sandboxed
    luaL_sandboxthread(ML);

    // now we can compile & run module on the new thread
    std::string bytecode = Luau::compile(resolvedRequire.sourceCode, copts());
    if (luau_load(ML, resolvedRequire.identifier.c_str(), bytecode.data(), bytecode.size(), 0) == 0)
    {
        if (codegen)
        {
            Luau::CodeGen::CompilationOptions nativeOptions;
            Luau::CodeGen::compile(ML, -1, nativeOptions);
        }

        int status = lua_resume(ML, L, 0);

        if (status == 0)
        {
            if (lua_gettop(ML) == 0)
                lua_pushstring(ML, "module must return a value");
            else if (!lua_istable(ML, -1) && !lua_isfunction(ML, -1))
                lua_pushstring(ML, "module must return a table or function");
        }
        else if (status == LUA_YIELD)
        {
            lua_pushstring(ML, "module can not yield");
        }
        else if (!lua_isstring(ML, -1))
        {
            lua_pushstring(ML, "unknown error while running module");
        }
    }

    // there's now a return value on top of ML; L stack: _MODULES ML
    lua_xmove(ML, L, 1);
    lua_pushvalue(L, -1);
    lua_setfield(L, -4, resolvedRequire.absolutePath.c_str());

    // L stack: _MODULES ML result
    return finishrequire(L);
}

static int lua_collectgarbage(lua_State* L)
{
    const char* option = luaL_optstring(L, 1, "collect");

    if (strcmp(option, "collect") == 0)
    {
        lua_gc(L, LUA_GCCOLLECT, 0);
        return 0;
    }

    if (strcmp(option, "count") == 0)
    {
        int c = lua_gc(L, LUA_GCCOUNT, 0);
        lua_pushnumber(L, c);
        return 1;
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

auto fw::load_script(state_t L, const path_t& path) -> std::expected<state_t, std::string> {
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
    constexpr std::array info = compile_time::to_array<Method>();
    auto found = rgs::find_if(info, [&namecall](decltype(info[0])& e) {
        return e.name == namecall;
    });
    if (found == rgs::end(info)) return -1;
    return static_cast<int16_t>(found->value);
}

auto fw::setup_state() -> state_owner_t {
    auto L = luaL_newstate();
    //lua_callbacks(L)->useratom = user_atom;
    if (codegen) Luau::CodeGen::create(L);
    luaL_openlibs(L);
    static const luaL_Reg funcs[] = {
        {"loadstring", lua_loadstring},
        {"require", lua_require},
        {"collectgarbage", lua_collectgarbage},
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

