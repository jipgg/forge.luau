#include "common.hpp"
#include "NamedAtom.hpp"
#include "util/lua.hpp"
using state = lua_State*;
using named = NamedAtom;

TYPE_CONFIG (HttpClient) {
    .type = "httpclient",
    .on_setup = [](state L) {
        using props = Properties<HttpClient>;
        using ref = HttpClient&;
        using cnst = const HttpClient&;
        props::add("host", [](state L, cnst self) {
            return lua::push(L, self.host());
        });
        props::add("isvalid", [](state L, cnst self) {
            return lua::push(L, self.is_valid());
        });
        props::add("port", [](state L, cnst self) {
            return lua::push(L, self.port());
        });
        props::add("encodeurl", nullptr, [](state L, ref self) {
            self.set_url_encode(luaL_checkboolean(L, 2));
        });
        props::add("keepalive", nullptr, [](state L, ref self) {
            self.set_keep_alive(luaL_checkboolean(L, 2));
        });
        props::add("connectiontimeout", nullptr, [](state L, ref self) {
            self.set_connection_timeout(std::chrono::milliseconds(luaL_checkinteger(L, 2)));
        });
        props::add("maxtimeout", nullptr, [](state L, ref self) {
            self.set_max_timeout(std::chrono::milliseconds(luaL_checkinteger(L, 2)));
        });
        props::add("readtimeout", nullptr, [](state L, ref self) {
            self.set_read_timeout(std::chrono::milliseconds(luaL_checkinteger(L, 2)));
        });
        props::add("writetimeout", nullptr, [](state L, ref self) {
            self.set_write_timeout(std::chrono::milliseconds(luaL_checkinteger(L, 2)));
        });
    },
    .namecall = [](lua_State* L) -> int {
        auto& self = Type<HttpClient>::to(L, 1);
        auto [atom, name] = lua::namecall_atom<NamedAtom>(L);
        switch (atom) {
            case NamedAtom::get: {
                auto r = self.Get(luaL_checkstring(L, 2));
                if (!r) return lua::push_tuple(L, lua::nil, std::format("error occurred ({})", static_cast<int>(r.error())));
                Type<HttpResponse>::make(L, std::move(*r));
                return 1;
            }
            case named::stop:
                self.stop();
                return lua::none;
            default:
                luaL_errorL(L, "invalid namecall %s", name);
        }
        luaL_errorL(L, "invalid namecall %s", name);
    },
    .index = Properties<HttpClient>::index,
    .newindex = Properties<HttpClient>::newindex,
};


TYPE_CONFIG (HttpResponse) {
    .type = "httpresponse",
    .on_setup = [](lua_State* L) {
        using props = Properties<HttpResponse>;
        using cnst = const HttpResponse&;
        props::add("body", [](state L, cnst self) {
            return lua::push(L, self.body);
        });
        props::add("status", [](state L, cnst self) {
            return lua::push(L, self.status);
        });
        props::add("location", [](state L, cnst self) {
            return lua::push(L, self.location);
        });
        props::add("version", [](state L, cnst self) {
            return lua::push(L, self.version);
        });
        props::add("reason", [](state L, cnst self) {
            return lua::push(L, self.reason);
        });
    },
    .namecall = [](lua_State* L) -> int {
        auto& self = Type<HttpResponse>::to(L, 1);
        auto [atom, name] = lua::namecall_atom<NamedAtom>(L);
        switch (atom) {
            case named::headers:
                lua_newtable(L);
                for(const auto& [key, header] : self.headers) {
                    lua::push_field(L, key, header);
                }
                return 1;
            default:
                luaL_errorL(L, "invalid namecall '%s'", name);
        }
    },
    .index = Properties<HttpResponse>::index,
    .newindex = Properties<HttpResponse>::newindex,

};
