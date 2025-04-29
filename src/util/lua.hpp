#ifndef UTILITY_LUA_HPP
#define UTILITY_LUA_HPP
#include <concepts>
#include <utility>
#include <string_view>
#include <ranges>
#include <vector>
#include <string>
#include <cstddef>
#include <expected>
#include <span>
#include <lualib.h>
#include <luacode.h>
#include <luacodegen.h>
#include <memory>
// general luau utility
namespace lua {
using state = lua_State*;
using state_ptr = std::unique_ptr<lua_State, decltype(&lua_close)>;
constexpr auto none = 0;
struct nil_t{};
constexpr nil_t nil{};
using destructor = lua_Destructor;
using reg = luaL_Reg;
using function = lua_CFunction;
using compile_options = lua_CompileOptions;
using useratom = int16_t(*)(const char*, size_t);
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
struct new_state_options {
    bool codegen = true;
    bool openlibs = true;
    useratom useratom = nullptr;
    std::span<luaL_Reg> globals = {};
    bool sandbox = false;
};
inline auto new_state(new_state_options const& opt = {}) -> state_ptr {
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
    return state_ptr{L, lua_close};
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
template <std::ranges::sized_range type>
constexpr auto make_buffer(state L, type&& range) -> std::span<std::ranges::range_value_t<type>> {
    using value_t = std::ranges::range_value_t<type>;
    auto const size = std::ranges::size(range);
    void* p = lua_newbuffer(L, size * sizeof(value_t));
    auto data = static_cast<value_t*>(p);
    auto span = std::span<value_t>{data, data + size};
    std::ranges::copy(range, span.begin());
    return span;
}
template<class type = char>
constexpr auto make_buffer(state L, size_t size) -> std::span<type> {
    auto data = static_cast<type*>(lua_newbuffer(L, size * sizeof(type)));
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
inline auto push(state L, ::lua::nil_t const& nil) -> int {
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
template <class type>
concept can_push = requires (state L, type v) {
    ::lua::push(L, v);
};
template <can_push ...types>
constexpr auto push_tuple(state L, types&&...args) -> int {
    (push(L, std::forward<types>(args)),...);
    return sizeof...(args);
}
template <class from, class to>
concept castable_to = requires {
    static_cast<from>(to{});
};
template<castable_to<int> type = int>
constexpr auto namecall_atom(state L) -> std::pair<type, const char*> {
    auto atom = int{};
    auto name = lua_namecallatom(L, &atom);
    return {static_cast<type>(atom), name};
}
template <class type,  class ...params>
requires std::constructible_from<type, params&&...>
constexpr auto make_userdata(state L, params&&...args) -> type& {
    auto dtor = [](void* ud) {
        static_cast<type*>(ud)->~type();
    };
    auto ud = static_cast<type*>(lua_newuserdatadtor(L, sizeof(type), dtor));
    std::construct_at(ud, std::forward<params>(args)...);
    return *ud;
}
template <class type>
constexpr auto new_userdata(state L, type&& userdata) -> type& {
    return make_userdata<type>(L, std::forward<type>(userdata));
}
template <class type>
constexpr auto to_userdata(state L, int idx) -> type& {
    return *static_cast<type*>(lua_touserdata(L, idx));
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
}
#endif
