#pragma once
#include "lua.hpp"
#include <functional>
namespace detail {
inline int new_tag() {
    static int tag{};
    return ++tag;
}
template<class type>
constexpr void default_destructor(lua_State* L, void* userdata) {
    static_cast<type*>(userdata)->~type();
}
}
template <class type, lua_Destructor destructor = detail::default_destructor<type>>
struct basic_type {
    using self = type;
    inline static int const tag = detail::new_tag();
    static const char* name;
    static auto setup(lua_State* L, const luaL_Reg* metamethods) -> bool {
        bool const init = luaL_newmetatable(L, name);
        if (init) {
            luaL_register(L, nullptr, metamethods);
            lua_pushstring(L, name);
            lua_setfield(L, -2, "__type");
            lua_setuserdatadtor(L, tag, destructor);
        }
        lua_pop(L, 1);
        return init;
    }
    template<class ...Args>
    requires std::constructible_from<type, Args&&...>
    static auto make(lua_State* L, Args&&...args) -> type& {
        auto p = static_cast<type*>(lua_newuserdatatagged(L, sizeof(type), tag));
        std::construct_at(p, std::forward<Args>(args)...);
        luaL_getmetatable(L, name);
        lua_setmetatable(L, -2);
        return *p;
    }
    static auto push(lua_State* L, type const& v) -> int {
        make(L) = v;
        return 1;
    }
    static auto get(lua_State* L, int idx) -> type& {
        auto p = static_cast<type*>(lua_touserdatatagged(L, idx, tag));
        return *p;
    }
    static auto is_type(lua_State* L, int idx) -> bool {
        return lua_userdatatag(L, idx) == tag;
    }
    static auto check(lua_State* L, int idx) -> type& {
        if (not is_type(L, idx)) luaL_typeerrorL(L, idx, name);
        return get(L, idx);
    }
    static auto get_if(lua_State* L, int idx) -> type* {
        if (not is_type(L, idx)) return nullptr;
        return static_cast<type*>(lua_touserdatatagged(L, idx, tag));
    }
    static auto get_or(lua_State* L, int idx, std::function<type()> fn) -> type {
        auto v = get_if(L, idx);
        return v ? *v : fn();
    } 
    static auto get_or(lua_State* L, int idx, type const& value) -> type const& {
        auto v = get_if(L, idx);
        return v ? *v : value;
    }
};
#define generate_type_trivial(type, basic_type)\
auto new_##type(lua_State* L, type&& value) -> type& {\
    return basic_type::make(L, std::move(value));\
}\
auto check_##type(lua_State* L, int idx) -> type& {\
    return basic_type::check(L, idx);\
}\
auto as_##type(lua_State* L, int idx) -> type& {\
    return basic_type::get(L, idx);\
}\
template<>\
const char* basic_type::name{type##_typename}
