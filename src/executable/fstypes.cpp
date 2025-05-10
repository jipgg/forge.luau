#include "common.hpp"
#include <format>
#include <filesystem>
#include "NamedAtom.hpp"
#include "util/type_utility.hpp"
namespace fs = std::filesystem;

auto to_path(lua_State* L, int idx) -> Path {
    return Type<Path>::to_or(L, idx, [&]{return luaL_checkstring(L, idx);});
}
auto push_path(lua_State* L, Path const& path) -> int {
    return Type<Path>::push(L, path);
}
auto test_namecall_path(lua_State* L, Path& self, int atom)-> std::optional<int> {
    using named = NamedAtom;
    switch (static_cast<named>(atom)) {
        case named::subpaths:
            return push_directory_iterator(L, self, luaL_optboolean(L, 2, false));
        case named::string:
            return lua::push(L, self.string());
        case named::subpath:
            return Type<Path>::push(L, self / to_path(L, 2));
        case named::isabsolute:
            return lua::push(L, self.is_absolute());
        case named::isrelative:
            return lua::push(L, self.is_relative());
        case named::generic:
            return lua::push(L, self.generic_string());
        case named::clone:
            return Type<Path>::push(L, self);
        default: return std::nullopt;
    }
};

static auto init_properties() {
    using P = Properties<Path>;
    P::add("ext", [](auto L, auto const& self) {
        return lua::push(L, self.extension().string());
    }, [](auto L, Path& self) {
        self.replace_extension(luaL_checkstring(L, 2));
    });
    P::add("name", [](auto L, auto const& self) {
        return lua::push(L, self.filename().string());
    }, [](auto L, auto& self) {
        self.replace_filename(luaL_checkstring(L, 2));
    });
    P::add("stem", [](auto L, auto const& self) {
        return lua::push(L, self.stem().string());
    }, [](auto L, auto& self){
        auto ext = self.extension().string();
        self.replace_filename(luaL_checkstring(L, 2) + ext);
    });
    P::add("dir", [](auto L, Path const& self) {
        return Type<Path>::push(L, self.parent_path());
    }, [](auto L, Path& self) {
        self = to_path(L, 2) / self.filename();
    });
    P::add("type", [](auto L, Path const& self) {
        if (fs::is_directory(self)) return lua::push(L, "directory");
        else if (fs::is_regular_file(self)) return lua::push(L, "file");
        else if (fs::is_symlink(self)) return lua::push(L, "symlink");
        else return lua::push(L, "unknown");
    });
}
TYPE_CONFIG (Path) {
    .type = "path",
    .on_setup = [](lua_State* L) {init_properties();},
    .namecall = [](lua_State* L) {
        auto& self = Type<Path>::to(L, 1);
        const auto [atom, name] = lua::namecall_atom(L);
        if (auto v = test_namecall_path(L, self, atom)) return *v;
        else luaL_errorL(L, "invalid namecall '%s'", name);
    },
    .tostring = [](lua_State* L) {
        auto fmt = std::format("\"{}\"", Type<Path>::to(L, 1).string());
        lua_pushstring(L, fmt.c_str());
        return 1;
    },
    .index = Properties<Path>::index,
    .newindex = Properties<Path>::newindex,
    .div = [](lua_State* L) -> int {
        return Type<Path>::push(L, to_path(L, 1) / to_path(L, 2));
    },
};
