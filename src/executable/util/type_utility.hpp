#pragma once
#include "lua.hpp"
#include <functional>
#include <optional>
#define TYPE_CONFIG(T) template<> TypeConfig const Type<T>::config
namespace detail {
inline int new_tag() {
    static int tag{};
    return ++tag;
}
template<class T>
constexpr void default_destructor(lua_State* L, void* userdata) {
    static_cast<T*>(userdata)->~T();
}
}

struct TypeConfig {
    std::string type = "unknown";
    std::function<void(lua_State*)> on_setup;
    lua_CFunction namecall = nullptr;
    lua_CFunction tostring = nullptr;
    lua_CFunction call = nullptr;
    lua_CFunction index = nullptr;
    lua_CFunction newindex = nullptr;
    lua_CFunction div = nullptr;
    int tag = detail::new_tag();
    auto tname() const -> const char* {
        return type.c_str();
    }
};
template <class T, lua_Destructor Dtor = detail::default_destructor<T>>
struct Type {
    static const TypeConfig config;
    using Self = T;
    //inline static int const tag = detail::new_tag();
    static bool setup(lua_State* L) {
        bool const init = luaL_newmetatable(L, config.tname());
        if (init) {
            if (config.on_setup) config.on_setup(L);
            auto init_method = [&](auto name, auto fn) {
                if (not fn) return;
                lua_pushcfunction(L, fn, name);
                lua_setfield(L, -2, name);
            };
            init_method("__index", config.index);
            init_method("__newindex", config.newindex);
            init_method("__namecall", config.namecall);
            init_method("__tostring", config.tostring);
            init_method("__call", config.call);
            init_method("__div", config.div);
            lua_pushstring(L, config.tname());
            lua_setfield(L, -2, "__type");
            lua_setuserdatadtor(L, config.tag, Dtor);
        }
        lua_pop(L, 1);
        return init;
    }
    template<class ...V>
    requires std::constructible_from<T, V&&...>
    static auto make(lua_State* L, V&&...args) -> T& {
        auto p = static_cast<T*>(lua_newuserdatatagged(L, sizeof(T), config.tag));
        std::construct_at(p, std::forward<V>(args)...);
        luaL_getmetatable(L, config.tname());
        lua_setmetatable(L, -2);
        return *p;
    }
    static auto push(lua_State* L, T const& v) -> int {
        make(L) = v;
        return 1;
    }
    static auto to(lua_State* L, int idx) -> T& {
        auto p = static_cast<T*>(lua_touserdatatagged(L, idx, config.tag));
        return *p;
    }
    static auto is_type(lua_State* L, int idx) -> bool {
        return lua_userdatatag(L, idx) == config.tag;
    }
    static auto check(lua_State* L, int idx) -> T& {
        if (not is_type(L, idx)) luaL_typeerrorL(L, idx, config.tname());
        return to(L, idx);
    }
    static auto to_if(lua_State* L, int idx) -> T* {
        if (not is_type(L, idx)) return nullptr;
        return static_cast<T*>(lua_touserdatatagged(L, idx, config.tag));
    }
    static auto to_or(lua_State* L, int idx, std::function<T()> fn) -> T {
        auto v = to_if(L, idx);
        return v ? *v : fn();
    } 
    static auto to_or(lua_State* L, int idx, T const& value) -> T const& {
        auto v = to_if(L, idx);
        return v ? *v : value;
    }
};
template <class T>
struct Properties {
    using Setter = std::function<void(lua_State*, T&)>;
    using Getter = std::function<int(lua_State*, T const&)>;
    struct Property {
        std::function<int(lua_State*, T const&)> get;
        std::function<void(lua_State*, T&)> set;
    };
    inline static std::unordered_map<std::string, Property> properties{};
    static auto getter_for(std::string const& name) -> std::optional<Getter> {
        if (properties.contains(name) and properties[name].get) return properties[name].get;
        return std::nullopt;
    }
    static auto setter_for(std::string const& name) -> std::optional<Setter> {
        if (properties.contains(name) and properties[name].set) return properties[name].set;
        return std::nullopt;
    }
    static void add(std::string const& name, Getter get = {}, Setter set = {}) {
        properties.insert({name, Property{.get = get, .set = set}});
    }
    static auto index(lua_State* L) -> int {
        if (auto opt = getter_for(luaL_checkstring(L, 2))) {
            auto& self = Type<T>::to(L, 1);
            lua_remove(L, 2);
            return (*opt)(L, self);
        } else luaL_errorL(L, "no getter set");
    }
    static auto newindex(lua_State* L) -> int {
        if (auto opt = setter_for(luaL_checkstring(L, 2))) {
            auto& self = Type<T>::to(L, 1);
            lua_remove(L, 2);
            (*opt)(L, self);
            return lua::none;
        } else luaL_errorL(L, "no setter set");
    }
};
