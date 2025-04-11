#ifndef UTILITY_LUA_HPP
#define UTILITY_LUA_HPP
#include <concepts>
#include <utility>
#include <string_view>
#include <string>
#include <cstddef>
#include <expected>
#include <span>
#include <lualib.h>
#include <luacode.h>
#include <luacodegen.h>
#include <functional>
#include <memory>
// general luau utility
namespace luau {
using state_t = lua_State*;
using state_owner_t = std::unique_ptr<lua_State, decltype(&lua_close)>;
using copts_t = lua_CompileOptions;
constexpr auto none = 0;
struct nil_t{};
constexpr nil_t nil{};
namespace codegen {
constexpr auto create(state_t L) -> bool {
    auto const supported = luau_codegen_supported();
    if (supported) luau_codegen_create(L);
    return supported;
}
constexpr auto compile(state_t L, int idx) {
    luau_codegen_compile(L, idx);
} 
}
using useratom_callback_t = int16_t(*)(const char*, size_t);
struct new_state_options {
    bool codegen = true;
    bool openlibs = true;
    useratom_callback_t useratom = nullptr;
    std::span<luaL_Reg> globals = {};
    bool sandbox = false;
};
constexpr auto new_state(new_state_options const& opt = {}) -> state_owner_t {
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
    return state_owner_t{L, lua_close};
}
struct load_options {
    int env = 0;
    bool codegen = true;
};
inline auto load(state_t L, std::span<char const> bytecode, std::string const& chunkname, load_options const& opts = {}) -> std::expected<void, std::string> {
    auto ok = LUA_OK == luau_load(L, chunkname.c_str(), bytecode.data(), bytecode.size(), opts.env);
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
inline auto compile(std::string_view source, copts_t& options) -> std::vector<char> {
    auto outsize = size_t{};
    auto raw = luau_compile(source.data(), source.size(), &options, &outsize);
    auto bytecode = std::vector<char>{raw, raw + outsize};
    std::free(raw);
    return bytecode;
}

constexpr auto push(state_t L, std::string_view v) -> int {
    lua_pushlstring(L, v.data(), v.size());
    return 1;
}
constexpr auto push(state_t L, const std::string& v) -> int {
    lua_pushstring(L, v.c_str());
    return 1;
}
constexpr auto push(state_t L, ::luau::nil_t nil) -> int {
    lua_pushnil(L);
    return 1;
}
constexpr auto push(state_t L, lua_CFunction fn) -> int {
    lua_pushcfunction(L, fn, "anonymous");
    return 1;
}
constexpr auto push(state_t L, bool boolean) -> int {
    lua_pushboolean(L, boolean);
    return 1;
} 
constexpr auto push(state_t L, double v) -> int {
    lua_pushnumber(L, v);
    return 1;
}
constexpr auto push(state_t L, int v) -> int {
    lua_pushinteger(L, v);
    return 1;
}
template <class Ty>
concept Pushable = requires (state_t L, Ty&& v) {
    ::luau::push(L, std::forward<Ty>(v));
};
template <Pushable ...Tys>
constexpr auto push_values(state_t L, Tys&&...args) -> int {
    (push(L, std::forward<Tys>(args)),...);
    return sizeof...(args);
}
template <class From, class To>
concept Castable_to = requires {
    static_cast<From>(To{});
};
template<Castable_to<int> Ty = int>
constexpr auto namecall_atom(state_t L) -> std::pair<Ty, const char*> {
    int atom{};
    const char* name = lua_namecallatom(L, &atom);
    return {static_cast<Ty>(atom), name};
}
template <class Ty,  class ...Params>
requires std::constructible_from<Ty, Params&&...>
constexpr auto make_userdata(state_t L, Params&&...args) -> Ty& {
    auto dtor = [](auto ud) {static_cast<Ty*>(ud)->~Ty();};
    Ty* ud = static_cast<Ty*>(lua_newuserdatadtor(L, sizeof(Ty), dtor));
    std::construct_at(ud, std::forward<Params>(args)...);
    return *ud;
}
template <class Ty>
constexpr auto new_userdata(state_t L, Ty&& userdata) {
    return make_userdata<Ty>(L, std::forward<Ty>(userdata));
}
template <class Ty>
constexpr auto to_userdata(state_t L, int idx) -> Ty& {
    return *static_cast<Ty*>(lua_touserdata(L, idx));
}
inline std::string_view tostring(lua_State* L, int idx) {
    size_t len;
    const char* str = luaL_tolstring(L, idx, &len);
    return {str, len};
}
struct tostring_format_options {
    int start_index = 1;
    std::string_view separator = ", ";
};
constexpr auto tostring_tuple(lua_State* L, tostring_format_options opts = {}) -> std::string {
    const int top = lua_gettop(L);
    std::string message;
    for (int i{opts.start_index}; i <= top; ++i) {
        message.append(tostring(L, i)).append(opts.separator);
    }
    message.pop_back();
    message.back() = '\n';
    return message;
}
namespace detail {
    inline int new_tag() {
        static int tag{};
        return ++tag;
    }
}
template <class Ty>
struct generic_userdatatagged_builder{
    inline static const int tag = detail::new_tag();
    static const char* name;
    static auto register_type(state_t L, const luaL_Reg* metamethods) -> bool {
        bool const init = luaL_newmetatable(L, name);
        if (init) {
            luaL_register(L, nullptr, metamethods);
            lua_pushstring(L, name);
            lua_setfield(L, -2, "__type");
            lua_setuserdatadtor(L, tag, [](auto, auto data) {
                static_cast<Ty*>(data)->~Ty();
            });
        }
        lua_pop(L, 1);
        return init;
    }
    template<class ...Args>
    requires std::constructible_from<Ty, Args&&...>
    static auto make(state_t L, Args&&...args) -> Ty& {
        auto p = static_cast<Ty*>(lua_newuserdatatagged(L, sizeof(Ty), tag));
        std::construct_at(p, std::forward<Args>(args)...);
        luaL_getmetatable(L, name);
        lua_setmetatable(L, -2);
        return *p;
    }
    static auto push(state_t L, const Ty& v) -> int {
        make(L) = v;
        return 1;
    }
    static auto get(state_t L, int idx) -> Ty& {
        auto p = static_cast<Ty*>(lua_touserdatatagged(L, idx, tag));
        return *p;
    }
    static auto self(state_t L) -> Ty& {
        return get(L, 1);
    }
    static auto is_type(state_t L, int idx) -> bool {
        return lua_userdatatag(L, idx) == tag;
    }
    static auto check(state_t L, int idx) -> Ty& {
        if (not is_type(L, idx)) luaL_typeerrorL(L, idx, name);
        return get(L, idx);
    }
    static auto get_if(state_t L, int idx) -> Ty* {
        if (not is_type(L, idx)) return nullptr;
        return static_cast<Ty*>(lua_touserdatatagged(L, idx, tag));
    }
    static auto get_or(state_t L, int idx, std::function<Ty()> fn) -> Ty {
        auto v = get_if(L, idx);
        return v ? *v : fn();
    } 
    static auto get_or(state_t L, int idx, const Ty& value) -> const Ty& {
        auto v = get_if(L, idx);
        return v ? *v : value;
    }
};
}
#endif
