#include "common.hpp"
#include "NamedAtom.hpp"
#include "util/lua.hpp"

TYPE_CONFIG (HttpClient) {
    .type = "httpclient",
    .namecall = [](lua_State* L) -> int {
        auto& self = Type<HttpClient>::to(L, 1);
        auto [atom, name] = lua::namecall_atom<NamedAtom>(L);
        switch (atom) {
            case NamedAtom::get:
                try {
                    auto r = self.Get(luaL_checkstring(L, 2));
                    return r ? lua::push_tuple(L, r->status, r->body)
                        : lua::push_tuple(L, lua::nil, "request failed");
                } catch (const std::exception& e) {
                    return lua::push_tuple(L, lua::nil, e.what());
                }
                break;
            case NamedAtom::post:
                try {
                    auto path = luaL_checkstring(L, 2);
                    auto body = luaL_checkstring(L, 3);
                    auto r = self.Post(path, body, "application/json");
                    if (r) return lua::push_tuple(L, r->status, r->body);
                    else return lua::push_tuple(L, lua::nil, "request failed");
                } catch (const std::exception& e) {
                    return lua::push_tuple(L, lua::nil, e.what());
                }
                break;
            default:
                luaL_errorL(L, "invalid namecall %s", name);
        }
        luaL_errorL(L, "invalid namecall %s", name);
    },
};
