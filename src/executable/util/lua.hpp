#ifndef UTILITY_LUA_HPP
#define UTILITY_LUA_HPP
#include <concepts>
#include <utility>
#include <string_view>
#include <ranges>
#include <vector>
#include <string>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>
#include <lualib.h>
#include <luacode.h>
#include <luacodegen.h>
#include <memory>
// general luau utility
namespace lua {
using State = lua_State*;
using StateOwner = std::unique_ptr<lua_State, decltype(&lua_close)>;
struct Nil{};
constexpr Nil nil{};
struct None {
    constexpr operator int() const {return 0;}
    constexpr operator Nil() const {return nil;}
};
constexpr None none{};
using Destructor = lua_Destructor;
using Reg = luaL_Reg;
using Function = lua_CFunction;
using CompileOptions = lua_CompileOptions;
using Useratom = std::int16_t(*)(const char*, size_t);
namespace codegen {
inline auto create(State L) -> bool {
    auto const supported = luau_codegen_supported();
    if (supported) luau_codegen_create(L);
    return supported;
}
inline auto compile(State L, int idx) {
    luau_codegen_compile(L, idx);
} 
}
struct NewStateOptions {
    bool codegen = true;
    bool openlibs = true;
    Useratom useratom = nullptr;
    std::span<luaL_Reg> globals = {};
    bool sandbox = false;
};
inline auto new_state(NewStateOptions const& opt = {}) -> StateOwner {
    auto L = luaL_newstate();
    if (opt.useratom) lua_callbacks(L)->useratom = opt.useratom;
    if (opt.codegen) codegen::create(L);
    if (opt.openlibs) luaL_openlibs(L);
    if (not opt.globals.empty()) {
        lua_pushvalue(L, LUA_GLOBALSINDEX);
        for (auto const& [name, fn] : opt.globals) {
            if (not name or not fn) continue; 
            lua_pushcfunction(L, fn, name);
            lua_setfield(L, -2, name);
        }
        lua_pop(L, 1);
    }
    return StateOwner{L, lua_close};
}
struct LoadOptions {
    int env = 0;
    bool codegen = true;
    std::string chunkname = "anonymous";
};
inline auto load(State L, std::span<char const> bytecode, LoadOptions const& opts = {}) -> std::expected<void, std::string> {
    auto ok = LUA_OK == luau_load(L, opts.chunkname.c_str(), bytecode.data(), bytecode.size(), opts.env);
    if (!ok) {
        std::string errmsg = lua_tostring(L, -1);
        lua_pop(L, 1);
        return std::unexpected(errmsg);
    }
    if (opts.codegen) {
        luau_codegen_compile(L, -1);
    }
    return {};
}
inline auto pcall(State L, int argn = 0, int resultn = 0, int errfn = 0) -> std::expected<void, std::string> {
    int r = lua_pcall(L, argn, resultn, errfn);
    if (r != LUA_OK) {
        auto err = std::string(lua_tostring(L, -1));
        lua_pop(L, 1);
        return std::unexpected(err);
    }
    return {};
}
inline auto compile(std::string_view source, CompileOptions options = {}) -> std::vector<char> {
    auto outsize = size_t{};
    auto raw = luau_compile(source.data(), source.size(), &options, &outsize);
    auto bytecode = std::vector<char>{raw, raw + outsize};
    std::free(raw);
    return bytecode;
}
inline auto compile_and_load(State L, std::string_view source, CompileOptions const& copts = {}, LoadOptions const& lopts = {}) -> std::expected<void, std::string> {
    auto bytecode = compile(source, copts);
    return load(L, bytecode, lopts);
}
template <std::ranges::sized_range T>
constexpr auto make_buffer(State L, T&& range) -> std::span<std::ranges::range_value_t<T>> {
    using value_t = std::ranges::range_value_t<T>;
    auto const size = std::ranges::size(range);
    void* p = lua_newbuffer(L, size * sizeof(value_t));
    auto data = static_cast<value_t*>(p);
    auto span = std::span<value_t>{data, data + size};
    std::ranges::copy(range, span.begin());
    return span;
}
template<class T = char>
constexpr auto make_buffer(State L, size_t size) -> std::span<T> {
    auto data = static_cast<T*>(lua_newbuffer(L, size * sizeof(T)));
    return {data, data + size};
}

inline auto to_buffer(State L, int idx) -> std::span<char> {
    auto size = size_t{};
    auto data = static_cast<char*>(lua_tobuffer(L, idx, &size));
    return {data, data + size};
}

inline auto push(State L, std::string_view v) -> int {
    lua_pushlstring(L, v.data(), v.size());
    return 1;
}
inline auto push(State L, const char* string) -> int {
    lua_pushstring(L, string);
    return 1;
}
inline auto push(State L, std::string const& v) -> int {
    lua_pushstring(L, v.c_str());
    return 1;
}
inline auto push(State L, ::lua::Nil const& nil) -> int {
    lua_pushnil(L);
    return 1;
}
inline auto push(State L, lua_CFunction function) -> int {
    lua_pushcfunction(L, function, "anonymous c function");
    return 1;
}
inline auto push(State L, bool boolean) -> int {
    lua_pushboolean(L, boolean);
    return 1;
} 
inline auto push(State L, double v) -> int {
    lua_pushnumber(L, v);
    return 1;
}
inline auto push(State L, int v) -> int {
    lua_pushinteger(L, v);
    return 1;
}
template <class T>
concept CanPush = requires (State L, T v) {
    ::lua::push(L, v);
};
template <CanPush ...V>
constexpr auto push_tuple(State L, V&&...args) -> int {
    (push(L, std::forward<V>(args)),...);
    return sizeof...(args);
}
template <class T, class U>
concept CastableTo = requires {
    static_cast<T>(U{});
};
template<CastableTo<int> T = int>
constexpr auto namecall_atom(State L) -> std::pair<T, const char*> {
    auto atom = int{};
    auto name = lua_namecallatom(L, &atom);
    return {static_cast<T>(atom), name};
}
template <class T,  class ...V>
requires std::constructible_from<T, V&&...>
constexpr auto make_userdata(State L, V&&...args) -> T& {
    auto dtor = [](void* ud) {
        static_cast<T*>(ud)->~T();
    };
    auto ud = static_cast<T*>(lua_newuserdatadtor(L, sizeof(T), dtor));
    std::construct_at(ud, std::forward<V>(args)...);
    return *ud;
}
template <class T>
constexpr auto new_userdata(State L, T&& userdata) -> T& {
    return make_userdata<T>(L, std::forward<T>(userdata));
}
template <class T>
constexpr auto to_userdata(State L, int idx) -> T& {
    return *static_cast<T*>(lua_touserdata(L, idx));
}
inline auto tostring(State L, int idx) -> std::string_view {
    size_t len;
    const char* str = luaL_tolstring(L, idx, &len);
    return {str, len};
}
struct TostringFormatOptions {
    int start_index = 1;
    std::string_view separator = ", ";
};
inline auto tostring_tuple(State L, TostringFormatOptions const& opts = {}) -> std::string {
    const int top = lua_gettop(L);
    std::string message;
    for (int i{opts.start_index}; i <= top; ++i) {
        message.append(tostring(L, i)).append(opts.separator);
    }
    message.pop_back();
    message.back() = '\n';
    return message;
}
inline void pop(State L, int amount = 1) {
    lua_pop(L, amount);
}
}
#endif
