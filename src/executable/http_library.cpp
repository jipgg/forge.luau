#include "common.hpp"
#include <httplib.h>
#include "util/json_utility.hpp"
#include <print>

static auto client(lua_State* L) -> int {
    Type<HttpClient>::make(L, luaL_checkstring(L, 1));
    return 1;
}
static auto get(lua_State* L) -> int {
    auto client = httplib::Client{luaL_checkstring(L, 1)};
    auto r = client.Get(luaL_checkstring(L, 2));
    if (!r) return lua::push_tuple(L, lua::nil, "request failed");
    if (r->get_header_value("Content-Type").contains("application/json")) {
        lua::push(L, r->status);
        util::push_json_as_table(L, r->body);
        return 2;
    }
    return lua::push_tuple(L, r->status, r->body);
}
static auto post(lua_State* L) -> int {
    auto client = httplib::Client{luaL_checkstring(L, 1)};
    std::string body{};
    if (lua_istable(L, 2)) {
        body = util::table_to_json(L, 3).dump();
    } else {
        body = luaL_checkstring(L, 3);
    }
    auto r = client.Post(luaL_checkstring(L, 2), body, "application/json");
    if (r) {
        lua::push(L, r->status);
        util::push_json_as_table(L, r->body);
        return 2;
    }
    return lua::push_tuple(L, lua::nil, "request failed");
}
void loader::http(lua_State* L, int idx) {
    lua::set_functions(L, idx, std::to_array<luaL_Reg>({
        {"client", client},
        {"get", get},
        {"post", post},
    }));
}
