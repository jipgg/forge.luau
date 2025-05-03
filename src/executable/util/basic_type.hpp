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
template <class type>
using setter = std::function<void(lua_State*, type&)>;
template <class type>
using getter = std::function<int(lua_State*, type const&)>;
template <class type>
struct property {
    getter<type> get;
    setter<type> set;
};

template <class type, lua_Destructor destructor = detail::default_destructor<type>>
struct basic_type {
    struct properties {
        inline static std::unordered_map<std::string, property<type>> registry{};
        static auto get(std::string const& name) -> std::optional<getter<type>> {
            if (registry.contains(name) and registry[name].get) return registry[name].get;
            return std::nullopt;
        }
        static auto set(std::string const& name) -> std::optional<setter<type>> {
            if (registry.contains(name) and registry[name].set) return registry[name].set;
            return std::nullopt;
        }
    };
    using self = type;
    inline static int const tag = detail::new_tag();
    static const char* name;
    static void add_property(std::string const& name, getter<type> get = {}, setter<type> set = {}) {
        properties::registry.insert({name, property<type>{.get = get, .set = set}});
    }
    static auto setup(lua_State* L, const luaL_Reg* metamethods) -> bool {
        bool const init = luaL_newmetatable(L, name);
        if (init) {
            if (not properties::registry.empty()) {
                lua_pushcfunction(L, [](lua_State* L) -> int {
                    if (auto opt = properties::get(luaL_checkstring(L, 2))) {
                        auto& self = get(L, 1);
                        lua_remove(L, 2);
                        return (*opt)(L, self);
                    } else luaL_errorL(L, "no getter set");
                }, "__index");
                lua_setfield(L, -2, "__index");
                lua_pushcfunction(L, [](lua_State* L) -> int {
                    if (auto opt = properties::set(luaL_checkstring(L, 2))) {
                        auto& self = get(L, 1);
                        lua_remove(L, 2);
                        (*opt)(L, self);
                        return lua::none;
                    } else luaL_errorL(L, "no setter set");
                }, "__newindex");
                lua_setfield(L, -2, "__newindex");
            }
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
const char* basic_type::name{type##_name}
