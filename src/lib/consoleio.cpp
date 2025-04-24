#include "common.hpp"
#include <iostream>
static auto read_line(state_t L) -> int {
    auto str = std::string{};
    std::cin >> str;
    luau::make_buffer(L, str);
    return 1;
}
static auto read(state_t L) -> int {
    if (lua_isnone(L, 1)) {
        auto str = std::string{};
        std::cin >> str;
        luau::make_buffer(L, str);
        return 1;
    }
    if (lua_isbuffer(L, 1)) {
        auto buf = luau::to_buffer(L, 1);
        std::cin.read(&buf.front(), buf.size());
        return luau::none;
    }
    auto const count = luaL_checkinteger(L, 1);
    auto buf = luau::make_buffer(L, count);
    std::cin.read(&buf.front(), buf.size());
    return 1;
}
static auto scan(state_t L) -> int {
    auto str = std::string{};
    std::cin >> str;
    return luau::push(L, str);
}
static auto write(state_t L) -> int {
    std::cout << luaL_checkstring(L, 1);
    return luau::none;
}
static auto err_write(state_t L) -> int {
    std::cerr << luaL_checkstring(L, 1);
    return luau::none;
}

template<>
void lib::consoleio::open(state_t L, library_config config) {
    const luaL_Reg consoleio[] = {
        {"write", write},
        {"read", read},
        {"scan", scan},
        {"err_write", err_write},
        {nullptr, nullptr}
    };
    config.apply(L, consoleio);
}
