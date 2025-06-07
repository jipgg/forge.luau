#pragma once
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

namespace lua {
using state = lua_State*;
using state_owner = std::unique_ptr<lua_State, decltype(&lua_close)>;
struct nil_t{};
constexpr nil_t nil{};
struct none_t {
    constexpr operator int() const {return 0;}
    constexpr operator nil_t() const {return nil;}
};
constexpr none_t none{};
using destructor = lua_Destructor;
using reg = luaL_Reg;
using function = lua_CFunction;
using compile_options = lua_CompileOptions;
using useratom = std::int16_t(*)(const char*, size_t);
namespace codegen {
inline auto create(state L) -> bool {
    auto const supported = luau_codegen_supported();
    if (supported) luau_codegen_create(L);
    return supported;
}
inline auto compile(state L, int idx) {
    luau_codegen_compile(L, idx);
} 
}
struct state_options {
    bool codegen = true;
    bool openlibs = true;
    useratom useratom = nullptr;
    std::span<luaL_Reg> globals = {};
    bool sandbox = false;
};
inline auto new_state(state_options const& opt = {}) -> state_owner {
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
    return state_owner{L, lua_close};
}
struct load_options {
    int env = 0;
    bool codegen = true;
    std::string chunkname = "anonymous";
};
inline auto load(state L, std::span<char const> bytecode, load_options const& opts = {}) -> std::expected<void, std::string> {
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
inline auto pcall(state L, int argn = 0, int resultn = 0, int errfn = 0) -> std::expected<void, std::string> {
    int r = lua_pcall(L, argn, resultn, errfn);
    if (r != LUA_OK) {
        auto err = std::string(lua_tostring(L, -1));
        lua_pop(L, 1);
        return std::unexpected(err);
    }
    return {};
}
inline auto compile(std::string_view source, compile_options options = {}) -> std::vector<char> {
    auto outsize = size_t{};
    auto raw = luau_compile(source.data(), source.size(), &options, &outsize);
    auto bytecode = std::vector<char>{raw, raw + outsize};
    std::free(raw);
    return bytecode;
}
inline auto compile_and_load(state L, std::string_view source, compile_options const& copts = {}, load_options const& lopts = {}) -> std::expected<void, std::string> {
    auto bytecode = compile(source, copts);
    return load(L, bytecode, lopts);
}
template <std::ranges::sized_range T>
constexpr auto make_buffer(state L, T&& range) -> std::span<std::ranges::range_value_t<T>> {
    using value_t = std::ranges::range_value_t<T>;
    auto const size = std::ranges::size(range);
    void* p = lua_newbuffer(L, size * sizeof(value_t));
    auto data = static_cast<value_t*>(p);
    auto span = std::span<value_t>{data, data + size};
    std::ranges::copy(range, span.begin());
    return span;
}
template<typename T = char>
constexpr auto make_buffer(state L, size_t size) -> std::span<T> {
    auto data = static_cast<T*>(lua_newbuffer(L, size * sizeof(T)));
    return {data, data + size};
}

inline auto to_buffer(state L, int idx) -> std::span<char> {
    auto size = size_t{};
    auto data = static_cast<char*>(lua_tobuffer(L, idx, &size));
    return {data, data + size};
}

inline auto push(state L, std::string_view v) -> int {
    lua_pushlstring(L, v.data(), v.size());
    return 1;
}
inline auto push(state L, const char* string) -> int {
    lua_pushstring(L, string);
    return 1;
}
inline auto push(state L, std::string const& v) -> int {
    lua_pushstring(L, v.c_str());
    return 1;
}
inline auto push(state L, std::wstring_view sv) -> int {
    auto str = sv
        | std::views::transform([](wchar_t e) {return static_cast<char>(e);})
        | std::ranges::to<std::string>();
    lua_pushstring(L, str.c_str());
    return 1;
}
inline auto push(state L, nil_t const& nil) -> int {
    lua_pushnil(L);
    return 1;
}
inline auto push(state L, lua_CFunction function) -> int {
    lua_pushcfunction(L, function, "anonymous c function");
    return 1;
}
inline auto push(state L, bool boolean) -> int {
    lua_pushboolean(L, boolean);
    return 1;
} 
inline auto push(state L, double v) -> int {
    lua_pushnumber(L, v);
    return 1;
}
inline auto push(state L, int v) -> int {
    lua_pushinteger(L, v);
    return 1;
}
template<typename T>
concept can_push = requires (state L, T v) {
    push(L, v);
};
template<can_push ...V>
constexpr auto push_tuple(state L, V&&...args) -> int {
    (push(L, std::forward<V>(args)),...);
    return sizeof...(args);
}
template<can_push T>
constexpr void set_field(state L, const char* name, T&& value, int idx = -2) {
    push(L, std::forward<T>(value));
    lua_setfield(L, -2, name);
}
constexpr auto set_field(state L, const std::string& name, auto&& value, int idx = -2) {
    return set_field(L, name.c_str(), std::forward<decltype(value)>(value));
}
template<class T, class U>
concept castable_to = requires {
    static_cast<T>(U{});
};
template<castable_to<int> T = int>
constexpr auto namecall_atom(state L) -> std::pair<T, const char*> {
    auto atom = int{};
    auto name = lua_namecallatom(L, &atom);
    return {static_cast<T>(atom), name};
}
template <typename T,  typename ...V>
requires std::constructible_from<T, V&&...>
constexpr auto make_userdata(state L, V&&...args) -> T& {
    auto dtor = [](void* ud) {
        static_cast<T*>(ud)->~T();
    };
    auto ud = static_cast<T*>(lua_newuserdatadtor(L, sizeof(T), dtor));
    std::construct_at(ud, std::forward<V>(args)...);
    return *ud;
}
template <typename T>
constexpr auto new_userdata(state L, T&& userdata) -> T& {
    return make_userdata<T>(L, std::forward<T>(userdata));
}
template <typename T>
constexpr auto to_userdata(state L, int idx) -> T& {
    return *static_cast<T*>(lua_touserdata(L, idx));
}
inline auto tostring(state L, int idx) -> std::string_view {
    size_t len;
    const char* str = luaL_tolstring(L, idx, &len);
    return {str, len};
}
struct tostring_format_options {
    int start_index = 1;
    std::string_view separator = ", ";
};
inline auto tostring_tuple(state L, tostring_format_options const& opts = {}) -> std::string {
    const int top = lua_gettop(L);
    std::string message;
    for (int i{opts.start_index}; i <= top; ++i) {
        message.append(tostring(L, i)).append(opts.separator);
    }
    message.pop_back();
    message.back() = '\n';
    return message;
}
inline void pop(state L, int amount = 1) {
    lua_pop(L, amount);
}
inline void set_functions(lua_State* L, int idx, std::span<const luaL_Reg> fns) {
    for (auto const& entry : fns) {
        lua_pushcfunction(L, entry.func, entry.name);
        lua_setfield(L, idx, entry.name);
    }
}
inline void push_function_table(lua_State* L, std::span<const luaL_Reg> fns) {
    lua_newtable(L);
    set_functions(L, -2, fns);
}
template <typename T>
concept compile_options_ish = requires (T v) {
    v.optimizationLevel;
    v.debugLevel;
    v.typeInfoLevel;
    v.coverageLevel;
    v.userdataTypes;
};
}
