#include "common.hpp"
#include <lualib.h>
#include <filesystem>
#include "utility/luau.hpp"
#include <string_view>
#include <cassert>
#include <expected>
#ifdef _WIN32
#include <Windows.h>
#include <ShlObj.h>
#endif
namespace fs = std::filesystem;

// util
static void error_on_code(state_t L, const std::error_code& ec) {
    if (ec) luaL_errorL(L, "%s", ec.message().c_str());
}
static auto get_home_path() -> std::expected<fs::path, std::string> {
    constexpr auto errmsg = "Failed to get HOME directory.";
    #ifdef _WIN32
        PWSTR path{};
        if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Profile, 0, nullptr, &path))) {
            auto home = std::wstring{path};
            CoTaskMemFree(path);
            return fs::path(home);
        } else return std::unexpected(errmsg);
    #else
        if (auto home_dir = std::getenv("HOME")) {
            return fs::path(home_dir);
        } else return std::unexpected(errmsg);
    #endif
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
static auto touch(fs::path const& path) -> bool {
    auto const first_touch = not fs::exists(path); 
    if (first_touch) auto _ = std::fstream{path, std::ios::out};
    else auto _ = std::fstream{path}; 
    return first_touch;
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
static auto directory_iterator(state_t L) -> int {
    const fs::path& directory = to_path(L, 1);
    const bool recursive = luaL_optboolean(L, 2, false);
    return push_directory_iterator(L, directory, recursive);
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
    return path_builder_t::push(L, fs::current_path());
}
static auto exists(state_t L) -> int {
    return luau::push(L, fs::exists(to_path(L, 1)));
}
static auto is_directory(state_t L) -> int {
    return luau::push(L, fs::is_directory(to_path(L, 1)));
}
static auto is_file(state_t L) -> int {
    lua_pushboolean(L, fs::is_regular_file(to_path(L, 1)));
    return 1;
}
static auto temp_directory_path(state_t L) -> int {
    std::error_code ec{};
    auto path = fs::temp_directory_path(ec);
    if (ec) luaL_errorL(L, "%s", ec.message().c_str());
    return path_builder_t::push(L, path);
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
    return path_builder_t::push(L, path);
}
static auto canonical(state_t L) -> int {
    std::error_code ec{};
    auto path = fs::canonical(to_path(L, 1), ec);
    error_on_code(L, ec);
    return path_builder_t::push(L, path);
}
static auto absolute(state_t L) -> int {
    std::error_code ec{};
    auto path = fs::absolute(to_path(L, 1), ec);
    error_on_code(L, ec);
    return path_builder_t::push(L, path);
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
    return path_builder_t::push(L, luaL_checkstring(L, 1));
}
static auto find_in_environment(state_t L) -> int {
    auto env = std::getenv(luaL_checkstring(L, 1));
    if (not env) return luau::push(L, luau::nil);
    else return path_builder_t::push(L, env);
}
static auto read_symlink(state_t L) -> int {
    auto ec = std::error_code{};
    auto const path = fs::read_symlink(to_path(L, 1), ec);
    error_on_code(L, ec);
    return path_builder_t::push(L, path);
}
static auto home_path(state_t L) -> int {
    auto const home = get_home_path();
    if (not home) luaL_errorL(L, "%s", home.error().c_str());
    return path_builder_t::push(L, home.value());
}
static auto open_file(state_t L) -> int {
    auto const path = to_path(L, 1);
    auto& file = file_builder_t::make(L);
    file.open(path);
    if (!file.is_open()) luaL_errorL(L, "failed to open file '%s'", path.string().c_str());
    return 1;
}
static auto create_file(state_t L) -> int {
    auto const path = to_path(L, 1);
    if (not touch(path)) luaL_errorL(L, "file '%s' already exists", path.string().c_str());
    file_builder_t::make(L, path);
    return 1;
}
static auto touch_file(state_t L) -> int {
    return luau::push(L, touch(to_path(L, 1)));
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
        {"is_file", is_file},
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
        {"find_in_environment", find_in_environment},
        {"read_symlink", read_symlink},
        {"home_path", home_path},
        {"open_file", open_file},
        {"create_file", create_file},
        {"touch_file", touch_file},
        {nullptr, nullptr}
    };
    config.apply(L, filesystem);
}
