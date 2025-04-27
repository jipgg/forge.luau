#include "common.hpp"
#include <format>
#include <filesystem>
#include "utility/luau.hpp"
#include "method.hpp"
namespace fs = std::filesystem;
using util = path::util;

template<>
const char* util::name{"__path"};
auto to_path(state_t L, int idx) -> path_t {
    return util::get_or(L, idx, [&]{return luaL_checkstring(L, idx);});
}
template<>
auto path::name() -> const char* {
    return util::name;
}
template<>
auto path::namecall_if(state_t L, type& self, int atom)-> std::optional<int> {
    switch (static_cast<method>(atom)) {
        case method::clone:
            return util::push(L, self);
        case method::iterate_children:
            return push_directory_iterator(L, self, false);
        case method::iterate_descendants:
            return push_directory_iterator(L, self, true);
        case method::is_directory:
            return luau::push(L, fs::is_directory(self));
        case method::is_file:
            return luau::push(L, fs::is_regular_file(self));
        case method::is_symlink:
            return luau::push(L, fs::is_symlink(self));
        case method::string:
            return luau::push(L, self.string());
        case method::extension:
            return util::push(L, self.extension());
        case method::has_extension:
            return luau::push(L, self.has_extension());
        case method::parent:
            return util::push(L, self.parent_path());
        case method::child:
            return util::push(L, self / to_path(L, 2));
        case method::is_absolute:
            return luau::push(L, self.is_absolute());
        case method::is_relative:
            return luau::push(L, self.is_relative());
        case method::filename:
            return util::push(L, self.filename());
        case method::generic_string:
            return luau::push(L, self.generic_string());
        case method::has_filename:
            return luau::push(L, self.has_filename());
        case method::replace_extension:
            return util::push(L, type(self).replace_extension(to_path(L, 2)));
        case method::replace_filename:
            return util::push(L, type(self).replace_filename(luaL_checkstring(L, 2)));
        case method::remove_filename:
            return util::push(L, type(self).remove_filename());
        case method::remove_extension:
            return util::push(L, type(self).replace_extension());
        case method::children: {
            lua_newtable(L);
            int n{1};
            for (auto const& entry : fs::directory_iterator(self)) {
                util::make(L, entry.path());
                lua_rawseti(L, -2, n++);
            }
            return 1;
        }
        case method::descendants: {
            lua_newtable(L);
            int n{1};
            for (auto const& entry : fs::recursive_directory_iterator(self)) {
                util::make(L, entry.path());
                lua_rawseti(L, -2, n++);
            }
            return 1;
        }
        default: return std::nullopt;
    }
};

static auto mt_namecall(state_t L) -> int {
    auto& self = util::self(L);
    const auto [atom, name] = luau::namecall_atom(L);
    if (auto v = path::namecall_if(L, self, atom)) return *v;
    else luaL_errorL(L, "invalid namecall '%s'", name);
}
static auto mt_tostring(state_t L) -> int {
    auto fmt = std::format("\"{}\"", util::self(L).string());
    lua_pushstring(L, fmt.c_str());
    return 1;
}
static auto mt_div(state_t L) -> int {
    return util::push(L, to_path(L, 1) / to_path(L, 2));
}

template<>
auto path::init(state_t L) -> void {
    const luaL_Reg metatable[] = {
        {"__namecall", mt_namecall},
        {"__tostring", mt_tostring},
        {"__div", mt_div},
        {nullptr, nullptr}
    };
    util::register_type(L, metatable);
}
