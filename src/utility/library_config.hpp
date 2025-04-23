#ifndef UTILITY_LIBRARY_CONFIG_HPP
#define UTILITY_LIBRARY_CONFIG_HPP
#include <functional>
#include "lualib.h"

using library_loader_t = std::function<void(lua_State* L)>;
struct library_config {
    const char* name = nullptr;
    bool local = false;
    void apply(lua_State* L, library_loader_t loader) {
        if (local) {
            lua_newtable(L);
            loader(L);
            if (name) lua_setfield(L, -2, name);
            return;
        } else if (name) {
            lua_newtable(L);
            loader(L);
            lua_setglobal(L, name);
            return;
        } else {
            lua_pushvalue(L, LUA_GLOBALSINDEX);
            loader(L);
            lua_pop(L, 1);
            return;
        }
    }
    void apply(lua_State* L, const luaL_Reg* lib) {
        apply(L, [&lib](auto L) {
            luaL_register(L, nullptr, lib);
        });
    }
};
#endif
