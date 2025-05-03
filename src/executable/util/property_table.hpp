#pragma once
#include <unordered_map>
#include <functional>
#include <lua.h>
#include <string>
#include <optional>
template <class type>
using property_accessor = std::function<int(lua_State*, type&)>;
template <class type>
struct property {
    property_accessor<type> get;
    property_accessor<type> set;
};

template <class type>
struct property_table {
    static std::unordered_map<std::string, property<type>> properties;
    static auto get(std::string const& name) -> std::optional<property_accessor<type>> {
        if (properties.contains(name)) return properties[name].get;
        return std::nullopt;
    }
    static auto set(std::string const& name) -> std::optional<property_accessor<type>> {
        if (properties.contains(name)) return properties[name].set;
        return std::nullopt;
    }
};
