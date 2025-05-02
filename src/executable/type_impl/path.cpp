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
        case named::clone:
            return push_path(L, self);
        case named::child_iterator:
            return push_directory_iterator(L, self, false);
        case named::descendant_iterator:
            return push_directory_iterator(L, self, true);
        case named::is_directory:
            return lua::push(L, fs::is_directory(self));
        case named::is_file:
            return lua::push(L, fs::is_regular_file(self));
        case named::is_symlink:
            return lua::push(L, fs::is_symlink(self));
        case named::string:
            return lua::push(L, self.string());
        case named::extension:
            return type::push(L, self.extension());
        case named::has_extension:
            return lua::push(L, self.has_extension());
        case named::parent:
            return type::push(L, self.parent_path());
        case named::child:
            return type::push(L, self / to_path(L, 2));
        case named::is_absolute:
            return lua::push(L, self.is_absolute());
        case named::is_relative:
            return lua::push(L, self.is_relative());
        case named::filename:
            return type::push(L, self.filename());
        case named::generic_string:
            return lua::push(L, self.generic_string());
        case named::has_filename:
            return lua::push(L, self.has_filename());
        case named::replace_extension:
            return type::push(L, path(self).replace_extension(to_path(L, 2)));
        case named::replace_filename:
            return type::push(L, path(self).replace_filename(luaL_checkstring(L, 2)));
        case named::remove_filename:
            return type::push(L, path(self).remove_filename());
        case named::remove_extension:
            return type::push(L, path(self).replace_extension());
        case named::children: {
            lua_newtable(L);
            int n{1};
            for (auto const& entry : fs::directory_iterator(self)) {
                type::make(L, entry.path());
                lua_rawseti(L, -2, n++);
            }
            return 1;
        }
        case named::descendants: {
            lua_newtable(L);
            int n{1};
            for (auto const& entry : fs::recursive_directory_iterator(self)) {
                type::make(L, entry.path());
                lua_rawseti(L, -2, n++);
            }
            return 1;
        }
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

auto register_path(lua_State* L) -> void {
    const luaL_Reg metatable[] = {
        {"__namecall", mt_namecall},
        {"__tostring", mt_tostring},
        {nullptr, nullptr}
    };
    type::setup(L, metatable);
}
