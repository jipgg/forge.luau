#include "common.hpp"
#include <lualib.h>
#include <filesystem>
#include <string_view>
#include <cassert>
#include <expected>
#ifdef _WIN32
#include <Windows.h>
#include <ShlObj.h>
#endif
namespace fs = std::filesystem;

// util
static void error_on_code(lua_State* L, const std::error_code& ec) {
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
// filesystem
static auto remove(lua_State* L) -> int {
    std::error_code err{};
    int count{};
    if (luaL_optboolean(L, 2, false)) {
        count = fs::remove_all(to_path(L, 1), err);
    } else {
        count = fs::remove(to_path(L, 1), err);
    }
    error_on_code(L, err);
    return lua::push(L, count);
}
static auto rename(lua_State* L) -> int {
    std::error_code ec{};
    auto from = to_path(L, 1);
    auto to = to_path(L, 2);
    fs::rename(from, to, ec);
    if (ec) {
        luaL_errorL(L, "%s", ec.message().c_str());
    }
    return 0;
}
static auto newdir(auto L) -> int {
    std::error_code ec{};
    bool created{};
    auto const path = to_path(L, 1);
    if (luaL_optboolean(L, 2, false)) {
        created = fs::create_directories(path, ec);
    } else {
        created = fs::create_directory(path, ec);
    }
    error_on_code(L, ec);
    return lua::push(L, created);
}
static auto subpaths(auto L) -> int {
    const fs::path& directory = to_path(L, 1);
    const bool recursive = luaL_optboolean(L, 2, false);
    return push_directory_iterator(L, directory, recursive);
}
static auto currdir(auto L) -> int {
    return push_path(L, fs::current_path());
}
static auto exists(auto L) -> int {
    return lua::push(L, fs::exists(to_path(L, 1)));
}
static auto type(auto L) -> int {
    auto const path = to_path(L, 1);
    if (fs::is_directory(path)) return lua::push(L, "directory");
    else if (fs::is_symlink(path)) return lua::push(L, "symlink");
    else if (fs::is_regular_file(path)) return lua::push(L, "file");
    else return lua::push(L, "unknown");
}
static auto tmpdir(auto L) -> int {
    std::error_code ec{};
    auto path = fs::temp_directory_path(ec);
    if (ec) luaL_errorL(L, "%s", ec.message().c_str());
    return push_path(L, path);
}
static auto equivalent(auto L) -> int {
    std::error_code ec{};
    auto y = fs::equivalent(to_path(L, 1), to_path(L, 2) , ec);
    error_on_code(L, ec);
    return lua::push(L, y);
}
static auto canonical(auto L) -> int {
    std::error_code ec{};
    fs::path path;
    if (luaL_optboolean(L, 2, false)) {
        path = fs::weakly_canonical(to_path(L, 1), ec);
    } else {
        path = fs::canonical(to_path(L, 1), ec);
    }
    error_on_code(L, ec);
    return push_path(L, path);
}
static auto absolute(auto L) {
    std::error_code ec{};
    auto path = fs::absolute(to_path(L, 1), ec);
    error_on_code(L, ec);
    return push_path(L, path);
}
static auto copy(auto L) -> int {
    std::error_code ec{};
    const fs::path from = to_path(L, 1);
    const fs::path to = to_path(L, 2);
    auto opts = fs::copy_options::none;
    if (lua_isstring(L, 3)) {
        opts = to_copy_options(lua_tostring(L, 3));
    }
    if (fs::is_regular_file(from)) {
        fs::copy_file(from, to, opts, ec);
    } else if (fs::is_symlink(from)) {
        fs::copy_symlink(from, to, ec);
    } else {
        fs::copy(from, to, opts, ec);
    }
    error_on_code(L, ec);
    return lua::none;
}
static auto newsym(auto L) -> int {
    std::error_code ec{};
    auto to = to_path(L, 1);
    auto new_symlink = to_path(L, 2);
    if (fs::is_directory(to)) {
        fs::create_directory_symlink(to, new_symlink, ec);
    } else {
        fs::create_symlink(to, new_symlink, ec);
    }
    error_on_code(L, ec);
    return lua::none;
}
static auto path_create(auto L) -> int {
    return push_path(L, luaL_checkstring(L, 1));
}
static auto getenv(auto L) {
    auto env = std::getenv(luaL_checkstring(L, 1));
    if (not env) return lua::push(L, lua::nil);
    else return push_path(L, env);
}
static auto readsym(auto L) -> int {
    auto ec = std::error_code{};
    auto path = fs::read_symlink(to_path(L, 1), ec);
    error_on_code(L, ec);
    return push_path(L, path);
}
static auto homedir(auto L) -> int {
    auto home = get_home_path();
    if (not home) luaL_errorL(L, "%s", home.error().c_str());
    return push_path(L, home.value());
}
template <class Iterator>
static auto directory_iterator_closure(auto L) -> int {
    auto& it = lua::to_userdata<Iterator>(L, lua_upvalueindex(1));
    auto& entry = Type<Path>::to(L, lua_upvalueindex(2));
    const Iterator end{};
    if (it != end) {
        const std::filesystem::directory_entry& e = *it;
        entry = e.path();
        lua_pushvalue(L, lua_upvalueindex(2));
        ++it;
        return 1;
    }
    return 0;
}
auto push_directory_iterator(lua_State* L, Path const& directory, bool recursive) -> int {
    if (not fs::is_directory(directory)) {
        luaL_errorL(L, "path '%s' must be a directory", directory.string().c_str());
    }
    if (not recursive) {
        lua::make_userdata<fs::directory_iterator>(L, directory);
        push_path(L,{});
        lua_pushcclosure(L,
            directory_iterator_closure<fs::directory_iterator>,
            "directory_iterator",
            2
        );
    } else {
        lua::make_userdata<fs::recursive_directory_iterator>(L, directory);
        push_path(L,{});
        lua_pushcclosure(
            L,
            directory_iterator_closure<fs::recursive_directory_iterator>,
            "recursive_directory_iterator",
            2
        );
    }
    return 1;
}

void loader::filesystem(lua_State* L, int idx) {
    lua::set_functions(L, idx, std::to_array<luaL_Reg>({
        {"remove", remove},
        {"rename", rename},
        {"newdir", newdir},
        {"subpaths", subpaths},
        {"exists", exists},
        {"currdir", currdir},
        {"type", type},
        {"tmpdir", tmpdir},
        {"equivalent", equivalent},
        {"canonical", canonical},
        {"absolute", absolute},
        {"copy", copy},
        {"newsym", newsym},
        //{"path", path_create},
        {"getenv", getenv},
        {"readsym", readsym},
        {"homedir", homedir},
    }));
    lua_newtable(L);
    lua::set_functions(L, -2, std::to_array<luaL_Reg>({
        {"home", homedir},
        {"temp", tmpdir},
        {"current", currdir},
    }));
    if (luaL_newmetatable(L, "path_type")) {
        lua::set_functions(L, -2, std::to_array<luaL_Reg>({
            {"__call", [](lua_State* L) -> int {
                return push_path(L, luaL_checkstring(L, 2));
            }},
        }));
    }
    lua_setmetatable(L, -2);
    lua_setfield(L, -2, "path");
}

// void open_fslib(lua_State* L) {
//     const luaL_Reg filesystem[] = {
//         {"remove", remove},
//         {"rename", rename},
//         {"newdir", newdir},
//         {"subpaths", subpaths},
//         {"exists", exists},
//         {"currdir", currdir},
//         {"type", type},
//         {"tmpdir", tmpdir},
//         {"equivalent", equivalent},
//         {"canonical", canonical},
//         {"absolute", absolute},
//         {"copy", copy},
//         {"newsym", newsym},
//         {"path", path_create},
//         {"getenv", getenv},
//         {"readsym", readsym},
//         {"homedir", homedir},
//         {nullptr, nullptr}
//     };
//     luaL_register(L, "fs", filesystem);
// }
//
