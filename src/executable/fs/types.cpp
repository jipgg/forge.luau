#include "export.hpp"
#include <format>
#include <filesystem>
#include "NamedAtom.hpp"
#include "util/type_utility.hpp"
#include <lualib.h>
#include <string_view>
#include <ranges>
using wow::fs::path_t;

auto wow::fs::to_path(lua_State* L, int idx) -> path_t {
    return Type<path_t>::to_or(L, idx, [&]{return luaL_checkstring(L, idx);});
}
auto wow::fs::push_path(lua_State* L, const path_t& path) -> int {
    return Type<path_t>::push(L, path);
}
auto test_namecall_path(lua_State* L, path_t& self, int atom)-> std::optional<int> {
    using named = NamedAtom;
    switch (static_cast<named>(atom)) {
        case named::children:
            return wow::fs::push_directory_iterator(L, self, luaL_optboolean(L, 2, false));
        case named::string:
            return lua::push(L, self.string());
        case named::child:
            return Type<path_t>::push(L, self / wow::fs::to_path(L, 2));
        case named::isabsolute:
            return lua::push(L, self.is_absolute());
        case named::isrelative:
            return lua::push(L, self.is_relative());
        case named::generic:
            return lua::push(L, self.generic_string());
        case named::clone:
            return Type<wow::fs::path_t>::push(L, self);
        default: return std::nullopt;
    }
};

static auto init_properties() {
    using self_t = wow::fs::path_t;
    using type_t = Type<self_t>;
    using props_t = Properties<self_t>;
    props_t::add("extension", [](auto L, const self_t& self) {
        return lua::push(L, self.extension().string());
    }, [](auto L, self_t& self) {
        self.replace_extension(luaL_checkstring(L, 2));
    });
    props_t::add("filename", [](auto L, auto const& self) {
        return lua::push(L, self.filename().string());
    }, [](auto L, auto& self) {
        self.replace_filename(luaL_checkstring(L, 2));
    });
    props_t::add("stem", [](auto L, auto const& self) {
        return lua::push(L, self.stem().string());
    }, [](auto L, auto& self){
        auto ext = self.extension().string();
        self.replace_filename(luaL_checkstring(L, 2) + ext);
    });
    props_t::add("parent", [](auto L, const self_t& self) {
        return Type<self_t>::push(L, self.parent_path());
    }, [](auto L, self_t& self) {
        self = wow::fs::to_path(L, 2) / self.filename();
    });
    props_t::add("type", [](auto L, auto const& self) {
        if (std::filesystem::is_directory(self)) return lua::push(L, "directory");
        else if (std::filesystem::is_regular_file(self)) return lua::push(L, "file");
        else if (std::filesystem::is_symlink(self)) return lua::push(L, "symlink");
        else return lua::push(L, "unknown");
    });
    props_t::add("canonical", [](auto L, self_t const& self) {
        std::error_code ec{};
        auto& v = type_t::make(L, std::filesystem::canonical(self, ec));
        if (ec) {
            lua_pop(L, 1);
            luaL_errorL(L, "%s", ec.message().c_str());
        }
        return 1;
    });
    props_t::add("absolute", [](auto L, auto const& self) {
        return type_t::push(L, std::filesystem::absolute(self));
    });
    props_t::add("string", [](auto L, auto const& self) {
        return lua::push(L, self.string());
    });
    props_t::add("generic", [](auto L, auto const& self) {
        return lua::push(L, self.generic_string());
    });
    props_t::add("native", [](auto L, auto const& self) {
        return lua::push(L, self.native());
    });
    props_t::add("isrelative", [](auto L, auto const& self) {
        return lua::push(L, self.is_relative());
    });
    props_t::add("isabsolute", [](auto L, auto const& self) {
        return lua::push(L, self.is_absolute());
    });
}
TYPE_CONFIG (wow::fs::path_t) {
    .type = "path",
    .on_setup = [](lua_State* L) {init_properties();},
    .namecall = [](lua_State* L) {
        auto& self = Type<path_t>::to(L, 1);
        const auto [atom, name] = lua::namecall_atom(L);
        if (auto v = test_namecall_path(L, self, atom)) return *v;
        else luaL_errorL(L, "invalid namecall '%s'", name);
    },
    .tostring = [](lua_State* L) {
        auto fmt = std::format("\"{}\"", Type<path_t>::to(L, 1).string());
        lua_pushstring(L, fmt.c_str());
        return 1;
    },
    .index = Properties<path_t>::index,
    .newindex = Properties<path_t>::newindex,
    .div = [](lua_State* L) -> int {
        return Type<path_t>::push(L, wow::fs::to_path(L, 1) / wow::fs::to_path(L, 2));
    },
};
