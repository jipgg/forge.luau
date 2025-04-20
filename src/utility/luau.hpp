#ifndef UTILITY_LUA_HPP
#define UTILITY_LUA_HPP
#include <concepts>
#include <utility>
#include <string_view>
#include <ranges>
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
using cstr_t = const char*;
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
using useratom_callback_t = int16_t(*)(cstr_t, size_t);
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
    std::string const& chunkname = "anonymous";
};
inline auto load(state_t L, std::span<char const> bytecode, load_options const& opts = {}) -> std::expected<void, std::string> {
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
inline auto pcall(state_t L, int argn = 0, int resultn = 0, int errfn = 0) -> std::expected<void, std::string> {
    int r = lua_pcall(L, argn, resultn, errfn);
    if (r != LUA_OK) {
        auto err = std::string(lua_tostring(L, -1));
        lua_pop(L, 1);
        return std::unexpected(err);
    }
    return {};
}
inline auto compile(std::string_view source, lua_CompileOptions options = {}) -> std::vector<char> {
    auto outsize = size_t{};
    auto raw = luau_compile(source.data(), source.size(), &options, &outsize);
    auto bytecode = std::vector<char>{raw, raw + outsize};
    std::free(raw);
    return bytecode;
}
inline auto compile_and_load(state_t L, std::string_view source, lua_CompileOptions const& copts = {}, load_options const& lopts = {}) -> std::expected<void, std::string> {
    auto bytecode = compile(source, copts);
    return load(L, bytecode, lopts);
}
template <std::ranges::sized_range Range>
constexpr auto make_buffer(state_t L, Range&& range) -> std::span<std::ranges::range_value_t<Range>> {
    using value_t = std::ranges::range_value_t<Range>;
    auto const size = std::ranges::size(range);
    void* p = lua_newbuffer(L, size * sizeof(value_t));
    auto data = static_cast<value_t*>(p);
    auto span = std::span<value_t>{data, data + size};
    std::ranges::copy(range, span.begin());
    return span;
}
template<class Ty = char>
constexpr auto make_buffer(state_t L, size_t size) -> std::span<Ty> {
    auto data = static_cast<Ty*>(lua_newbuffer(L, size * sizeof(Ty)));
    return {data, data + size};
}

inline auto to_buffer(state_t L, int idx) -> std::span<char> {
    auto size = size_t{};
    auto data = static_cast<char*>(lua_tobuffer(L, idx, &size));
    return {data, data + size};
}

constexpr auto push(state_t L, std::string_view v) -> int {
    lua_pushlstring(L, v.data(), v.size());
    return 1;
}
constexpr auto push(state_t L, cstr_t v) -> int {
    lua_pushstring(L, v);
    return 1;
}
constexpr auto push(state_t L, std::string const& v) -> int {
    lua_pushstring(L, v.c_str());
    return 1;
}
constexpr auto push(state_t L, ::luau::nil_t const& nil) -> int {
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
constexpr auto namecall_atom(state_t L) -> std::pair<Ty, cstr_t> {
    auto atom = int{};
    auto name = lua_namecallatom(L, &atom);
    return {static_cast<Ty>(atom), name};
}
template <class Ty,  class ...Params>
requires std::constructible_from<Ty, Params&&...>
constexpr auto make_userdata(state_t L, Params&&...args) -> Ty& {
    auto dtor = [](auto ud) {static_cast<Ty*>(ud)->~Ty();};
    auto ud = static_cast<Ty*>(lua_newuserdatadtor(L, sizeof(Ty), dtor));
    std::construct_at(ud, std::forward<Params>(args)...);
    return *ud;
}
template <class Ty>
constexpr auto new_userdata(state_t L, Ty&& userdata) -> Ty& {
    return make_userdata<Ty>(L, std::forward<Ty>(userdata));
}
template <class Ty>
constexpr auto to_userdata(state_t L, int idx) -> Ty& {
    return *static_cast<Ty*>(lua_touserdata(L, idx));
}
inline auto tostring(lua_State* L, int idx) -> std::string_view {
    size_t len;
    const char* str = luaL_tolstring(L, idx, &len);
    return {str, len};
}
struct tostring_format_options {
    int start_index = 1;
    std::string_view separator = ", ";
};
constexpr auto tostring_tuple(lua_State* L, tostring_format_options const& opts = {}) -> std::string {
    const int top = lua_gettop(L);
    std::string message;
    for (int i{opts.start_index}; i <= top; ++i) {
        message.append(tostring(L, i)).append(opts.separator);
    }
    message.pop_back();
    message.back() = '\n';
    return message;
}
constexpr void pop(state_t L, int amount = 1) {
    lua_pop(L, amount);
}
class ref {
    std::shared_ptr<int> shared_;
public:
    ref(): shared_(nullptr) {}
    ref(state_t L, int idx): shared_(std::make_shared<int>(lua_ref(L, idx))) {}
    void unref() {shared_.reset();}
    auto is_nil() const -> bool {return shared_ == nullptr;}
    operator bool() {return is_nil();}
    ~ref() {unref();}
    auto push(state_t L) const -> int {
        if (is_nil()) return 0;
        lua_getref(L, *shared_);
        return 1;
    }
    auto rawequal(state_t L, int idx) const -> bool {
        lua_pushvalue(L, idx);
        push(L);
        auto eq = lua_rawequal(L, -1, -2);
        lua_pop(L, 2);
        return eq;
    }
    auto rawequal(state_t L, ref const& ref) const -> bool {
        ref.push(L);
        push(L);
        auto eq = lua_rawequal(L, -1, -2);
        lua_pop(L, 2);
        return eq;
    }
};
namespace detail {
    inline int new_tag() {
        static int tag{};
        return ++tag;
    }
}
template <class Ty>
struct generic_userdatatagged_builder{
    inline static int const tag = detail::new_tag();
    static cstr_t name;
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
    static auto push(state_t L, Ty const& v) -> int {
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
    static auto get_or(state_t L, int idx, Ty const& value) -> Ty const& {
        auto v = get_if(L, idx);
        return v ? *v : value;
    }
};
}
#endif
