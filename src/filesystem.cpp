#include "common.hpp"
#include <lualib.h>
#include <filesystem>
#include "utility/luau.hpp"
#include <string_view>
#include <cassert>
namespace fs = std::filesystem;

// util
static void error_on_code(state_t L, const std::error_code& ec) {
    if (ec) luaL_errorL(L, "%s", ec.message().c_str());
}
static auto to_copy_options(std::string_view str) -> fs::copy_options {
    if (str == "recursive") {
        return fs::copy_options::recursive;
    } else if (str == "update existing") {
        return fs::copy_options::update_existing;
    } else if (str == "skip existing") {
        return fs::copy_options::skip_existing;
    } else if (str == "create symlinks") {
        return fs::copy_options::create_symlinks;
    } else if (str == "copy symlinks") {
        return fs::copy_options::copy_symlinks;
    } else if (str == "skip symlinks") {
        return fs::copy_options::skip_symlinks;
    } else if (str == "overwrite existing") {
        return fs::copy_options::overwrite_existing;
    } else if (str == "directories only") {
        return fs::copy_options::directories_only;
    } else if (str == "create hard links") {
        return fs::copy_options::create_hard_links;
    } else {
        return fs::copy_options::none;
    }
}
// filesystem
static auto remove(state_t L) -> int {
    std::error_code err{};
    auto r = fs::remove(to_path(L, 1), err);
    error_on_code(L, err);
    return luau::push(L, r);
}
static auto rename(state_t L) -> int {
    std::error_code ec{};
    auto from = to_path(L, 1);
    auto to = to_path(L, 2);
    fs::rename(from, to, ec);
    if (ec) {
        luaL_errorL(L, "%s", ec.message().c_str());
    }
    return 0;
}
static auto create_directory(state_t L) -> int {
    std::error_code ec{};
    auto result = fs::create_directory(to_path(L, 1), ec);
    error_on_code(L, ec);
    return luau::push(L, result);
}
template <class Iterator>
static auto iterator_closure(state_t L) -> int {
    auto& it = luau::to_userdata<Iterator>(L, lua_upvalueindex(1));
    const Iterator end{};
    if (it != end) {
        const fs::directory_entry& entry = *it;
        auto path = entry.path();
        push_path(L, path);
        ++it;
        return 1;
    }
    return 0;
}
static auto directory_iterator(state_t L) -> int {
    const fs::path& directory = to_path(L, 1);
    const bool recursive = luaL_optboolean(L, 2, false);
    if (not recursive) {
        luau::make_userdata<fs::directory_iterator>(L, directory);
        lua_pushcclosure(
            L,
            iterator_closure<fs::directory_iterator>,
            "directory_iterator",
            1
        );
    } else {
        luau::make_userdata<fs::recursive_directory_iterator>(L, directory);
        lua_pushcclosure(
            L,
            iterator_closure<fs::recursive_directory_iterator>,
            "recursive_directory_iterator",
            1
        );
    }
    return 1;
}
static auto remove_all(state_t L) -> int {
    std::error_code ec{};
    auto count = fs::remove_all(to_path(L, 1), ec);
    if (ec) {
        luaL_errorL(L, "%s", ec.message().c_str());
    }
    lua_pushnumber(L, static_cast<double>(count));
    return 1;
} 
static auto current_path(state_t L) -> int {
    return push_path(L, fs::current_path());
}
static auto exists(state_t L) -> int {
    return luau::push(L, fs::exists(to_path(L, 1)));
}
static auto is_directory(state_t L) -> int {
    return luau::push(L, fs::is_directory(to_path(L, 1)));
}
static auto is_regular_file(state_t L) -> int {
    lua_pushboolean(L, fs::is_regular_file(to_path(L, 1)));
    return 1;
}
static auto temp_directory_path(state_t L) -> int {
    std::error_code ec{};
    auto path = fs::temp_directory_path(ec);
    if (ec) luaL_errorL(L, "%s", ec.message().c_str());
    return push_path(L, path);
}
static auto equivalent(state_t L) -> int {
    std::error_code ec{};
    auto y = fs::equivalent(to_path(L, 1), to_path(L, 2) , ec);
    error_on_code(L, ec);
    return luau::push(L, y);
}
static auto weakly_canonical(state_t L) -> int {
    std::error_code ec{};
    auto path = fs::weakly_canonical(to_path(L, 1), ec);
    error_on_code(L, ec);
    return push_path(L, path);
}
static auto canonical(state_t L) -> int {
    std::error_code ec{};
    auto path = fs::canonical(to_path(L, 1), ec);
    error_on_code(L, ec);
    return push_path(L, path);
}
static auto absolute(state_t L) -> int {
    std::error_code ec{};
    auto path = fs::absolute(to_path(L, 1), ec);
    error_on_code(L, ec);
    return push_path(L, path);
}
static auto copy(state_t L) -> int {
    std::error_code ec{};
    const fs::path from = to_path(L, 1);
    const fs::path to = to_path(L, 2);
    if (lua_isnoneornil(L, 3)) {
        fs::copy(from, to, ec);
    } else {
        auto options = to_copy_options(luaL_checkstring(L, 3));
        fs::copy(from, to, options, ec);
    }
    error_on_code(L, ec);
    return 0;
}
static auto copy_file(state_t L) -> int {
    std::error_code ec{};
    const fs::path from = to_path(L, 1);
    const fs::path to = to_path(L, 2);
    bool result{};
    if (lua_isnoneornil(L, 3)) {
        result = fs::copy_file(from, to, ec);
    } else {
        auto options = to_copy_options(luaL_checkstring(L, 3));
        result = fs::copy_file(from, to, options, ec);
    }
    error_on_code(L, ec);
    return luau::push(L, result);
}
static auto copy_symlink(state_t L) -> int {
    std::error_code ec{};
    const fs::path from = to_path(L, 1);
    const fs::path to = to_path(L, 2);
    fs::copy_symlink(from, to, ec);
    error_on_code(L, ec);
    return 0;
}
static auto create_symlink(state_t L) -> int {
    std::error_code ec{};
    fs::create_symlink(to_path(L, 1), to_path(L, 2), ec);
    error_on_code(L, ec);
    return luau::none;
}
static auto create_directory_symlink(state_t L) -> int {
    std::error_code ec{};
    fs::create_directory_symlink(to_path(L, 1), to_path(L, 2), ec);
    error_on_code(L, ec);
    return luau::none;
}
static auto create_directories(state_t L) -> int {
    std::error_code ec{};
    auto result = fs::create_directories(to_path(L, 1), ec);
    error_on_code(L, ec);
    return luau::push(L, result);
}
static auto path(state_t L) -> int {
    return push_path(L, luaL_checkstring(L, 1));
}

void open_filesystem(state_t L, library_config config) {
    const luaL_Reg filesystem[] = {
        {"remove", remove},
        {"remove_all", remove_all},
        {"rename", rename},
        {"create_directory", create_directory},
        {"directory_iterator", directory_iterator},
        {"exists", exists},
        {"current_path", current_path},
        {"is_directory", is_directory},
        {"is_regular_file", is_regular_file},
        {"temp_directory_path", temp_directory_path},
        {"equivalent", equivalent},
        {"weakly_canonical", weakly_canonical},
        {"canonical", canonical},
        {"absolute", absolute},
        {"copy", copy},
        {"copy_file", copy_file},
        {"copy_symlink", copy_symlink},
        {"create_symlink", create_symlink},
        {"create_directory_symlink", create_directory_symlink},
        {"create_directories", create_directories},
        {"path", path},
        {nullptr, nullptr}
    };
    config.apply(L, filesystem);
}
