#include "export.hpp"
#include <format>
#include <filesystem>
#include "named_atom.hpp"
#include "lua/typeutility.hpp"
#include <lualib.h>
using lib::fs::path;
using self = lib::fs::path;
using type = lua::type<path>;
using props = lua::properties<path>;

auto lib::fs::to_path(lua_State* L, int idx) -> path {
    return type::to_or(L, idx, [&]{return luaL_checkstring(L, idx);});
}
auto lib::fs::push_path(lua_State* L, const path& path) -> int {
    return type::push(L, path);
}
auto test_namecall_path(lua_State* L, path& self, int atom)-> std::optional<int> {
    using named = named_atom;
    switch (static_cast<named>(atom)) {
        case named::children:
            return lib::fs::push_directory_iterator(L, self, luaL_optboolean(L, 2, false));
        case named::string:
            return lua::push(L, self.string());
        case named::child:
            return type::push(L, self / lib::fs::to_path(L, 2));
        case named::isabsolute:
            return lua::push(L, self.is_absolute());
        case named::isrelative:
            return lua::push(L, self.is_relative());
        case named::generic:
            return lua::push(L, self.generic_string());
        case named::clone:
            return type::push(L, self);
        default: return std::nullopt;
    }
};

static auto init_properties() {
    props::add("extension", [](auto L, const self& self) {
        return lua::push(L, self.extension().string());
    }, [](auto L, self& self) {
        self.replace_extension(luaL_checkstring(L, 2));
    });
    props::add("filename", [](auto L, auto const& self) {
        return lua::push(L, self.filename().string());
    }, [](auto L, auto& self) {
        self.replace_filename(luaL_checkstring(L, 2));
    });
    props::add("stem", [](auto L, auto const& self) {
        return lua::push(L, self.stem().string());
    }, [](auto L, auto& self){
        auto ext = self.extension().string();
        self.replace_filename(luaL_checkstring(L, 2) + ext);
    });
    props::add("parent", [](auto L, const self& self) {
        return type::push(L, self.parent_path());
    }, [](auto L, self& self) {
        self = lib::fs::to_path(L, 2) / self.filename();
    });
    props::add("type", [](auto L, auto const& self) {
        if (std::filesystem::is_directory(self)) return lua::push(L, "directory");
        else if (std::filesystem::is_regular_file(self)) return lua::push(L, "file");
        else if (std::filesystem::is_symlink(self)) return lua::push(L, "symlink");
        else return lua::push(L, "unknown");
    });
    props::add("canonical", [](auto L, self const& self) {
        std::error_code ec{};
        auto& v = type::make(L, std::filesystem::canonical(self, ec));
        if (ec) {
            lua_pop(L, 1);
            luaL_errorL(L, "%s", ec.message().c_str());
        }
        return 1;
    });
    props::add("absolute", [](auto L, auto const& self) {
        return type::push(L, std::filesystem::absolute(self));
    });
    props::add("string", [](auto L, auto const& self) {
        return lua::push(L, self.string());
    });
    props::add("generic", [](auto L, auto const& self) {
        return lua::push(L, self.generic_string());
    });
    props::add("native", [](auto L, auto const& self) {
        return lua::push(L, self.native());
    });
    props::add("isrelative", [](auto L, auto const& self) {
        return lua::push(L, self.is_relative());
    });
    props::add("isabsolute", [](auto L, auto const& self) {
        return lua::push(L, self.is_absolute());
    });
}
TYPE_CONFIG (lib::fs::path) {
    .type = "path",
    .on_setup = [](lua_State* L) {init_properties();},
    .namecall = [](lua_State* L) {
        auto& self = type::to(L, 1);
        const auto [atom, name] = lua::namecall_atom(L);
        if (auto v = test_namecall_path(L, self, atom)) return *v;
        else luaL_errorL(L, "invalid namecall '%s'", name);
    },
    .tostring = [](lua_State* L) {
        auto fmt = std::format("\"{}\"", type::to(L, 1).string());
        lua_pushstring(L, fmt.c_str());
        return 1;
    },
    .index = props::index,
    .newindex = props::newindex,
    .div = [](lua_State* L) -> int {
        return type::push(L, lib::fs::to_path(L, 1) / lib::fs::to_path(L, 2));
    },
};
