#include "framework.hpp"
#include <lualib.h>
#include <filesystem>
#include <fstream>
#include <string_view>
namespace fs = std::filesystem;

static auto file_write_all_text(state_t L) -> int {
    fs::path destination = luaL_checkstring(L, 1);
    std::string contents = luaL_checkstring(L, 2);
    std::fstream file{destination};
    file << contents;
    return 0;
}
static auto remove(state_t L) -> int {
    std::error_code err{};
    if (not fs::remove(luaL_checkstring(L, 1), err)) {
        luaL_errorL(L, "%s", err.message().c_str());
    }
    return 0;
}
static auto rename(state_t L) -> int {
    std::error_code ec{};
    std::string_view from = luaL_checkstring(L, 1);
    std::string_view to = luaL_checkstring(L, 2);
    fs::rename(from, to, ec);
    if (ec) {
        luaL_errorL(L, "%s", ec.message().c_str());
    }
    return 0;
}
static auto new_directory(state_t L) -> int {
    std::error_code ec{};
    if (!fs::create_directory(luaL_checkstring(L, 1), ec)) {
        luaL_errorL(L, "%s", ec.message().c_str());
    }
    return 0;
}
template <class Iterator>
static auto iterator_closure(state_t L) -> int {
    //auto& it = *static_cast<Iterator*>(lua_touserdata(L, lua_upvalueindex(1)));
    auto& it = fw::as_userdata<Iterator>(L, lua_upvalueindex(1));
    const Iterator end{};
    if (it != end) {
        const fs::directory_entry& entry = *it;
        auto path = entry.path();
        lua_pushstring(L, path.string().c_str());
        ++it;
        return 1;
    }
    return 0;
}
static auto iterator(state_t L) -> int {
    const fs::path& directory = luaL_checkstring(L, 1);
    const bool recursive = luaL_optboolean(L, 2, false);
    if (not recursive) {
        fw::make_userdata<fs::directory_iterator>(L, luaL_checkstring(L, 1));
        lua_pushcclosure(
            L,
            iterator_closure<fs::directory_iterator>,
            "directory_iterator",
            1
        );
    } else {
        fw::make_userdata<fs::recursive_directory_iterator>(L, luaL_checkstring(L, 1));
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
    auto count = fs::remove_all(luaL_checkstring(L, 1), ec);
    if (ec) {
        luaL_errorL(L, "%s", ec.message().c_str());
    }
    lua_pushnumber(L, static_cast<double>(count));
    return 1;
} 
static auto current_path(state_t L) -> int {
    lua_pushstring(L, fs::current_path().string().c_str());
    return 1;
}

void filesystem::open(state_t L) {
    const luaL_Reg filesystem[] = {
        {"remove", remove},
        {"remove_all", remove_all},
        {"rename", rename},
        {"write_file", file_write_all_text},
        {"new_directory", new_directory},
        {"iterator", iterator},
        {"current_path", current_path},
        {nullptr, nullptr}
    };
    lua_newtable(L);
    luaL_register(L, nullptr, filesystem);
    lua_setglobal(L, "Fs");
}
