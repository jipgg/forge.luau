#include "common.hpp"
#include <format>
#include <filesystem>
#include "named_atom.hpp"
#include "util/basic_type.hpp"
namespace fs = std::filesystem;
using type = basic_type<path>;
generate_type_trivial(path, type);

auto to_path(lua_State* L, int idx) -> path {
    return type::get_or(L, idx, [&]{return luaL_checkstring(L, idx);});
}
auto push_path(lua_State* L, path const& path) -> int {
    return type::push(L, path);
}
auto test_namecall_path(lua_State* L, path& self, int atom)-> std::optional<int> {
    using named = named_atom;
    switch (static_cast<named>(atom)) {
        case named::subpaths:
            return push_directory_iterator(L, self, luaL_optboolean(L, 2, false));
        case named::string:
            return lua::push(L, self.string());
        case named::subpath:
            return type::push(L, self / to_path(L, 2));
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

static auto mt_namecall(lua_State* L) -> int {
    auto& self = type::get(L, 1);
    const auto [atom, name] = lua::namecall_atom(L);
    if (auto v = test_namecall_path(L, self, atom)) return *v;
    else luaL_errorL(L, "invalid namecall '%s'", name);
}
static auto mt_tostring(lua_State* L) -> int {
    auto fmt = std::format("\"{}\"", type::get(L, 1).string());
    lua_pushstring(L, fmt.c_str());
    return 1;
}
static auto init_properties() {
    type::add_property("ext", [](auto L, path const& self) {
        return lua::push(L, self.extension().string());
    }, [](auto L, path& self) {
        self.replace_extension(luaL_checkstring(L, 2));
    });
    type::add_property("name", [](auto L, path const& self) {
        return lua::push(L, self.filename().string());
    }, [](auto L, path& self) {
        self.replace_filename(luaL_checkstring(L, 2));
    });
    type::add_property("stem", [](auto L, path const& self) {
        return lua::push(L, self.stem().string());
    }, [](auto L, path& self){
        auto ext = self.extension().string();
        self.replace_filename(luaL_checkstring(L, 2) + ext);
    });
    type::add_property("dir", [](auto L, path const& self) {
        return type::push(L, self.parent_path());
    }, [](auto L, path& self) {
        self = to_path(L, 2) / self.filename();
    });
    type::add_property("type", [](auto L, path const& self) {
        if (fs::is_directory(self)) return lua::push(L, "directory");
        else if (fs::is_regular_file(self)) return lua::push(L, "file");
        else if (fs::is_symlink(self)) return lua::push(L, "symlink");
        else return lua::push(L, "unknown");
    });
}
static auto div(lua_State* L) {
    return type::push(L, to_path(L, 1) / to_path(L, 2));
}

auto load_path(lua_State* L) -> void {
    init_properties();
    const luaL_Reg metatable[] = {
        {"__namecall", mt_namecall},
        {"__tostring", mt_tostring},
        {"__div", div},
        {nullptr, nullptr}
    };
    type::setup(L, metatable);
}
