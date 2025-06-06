#include "export.hpp"
#include "NamedAtom.hpp"
#include "util/lua.hpp"
#include "util/type_utility.hpp"
using state_t = lua_State*;
using named_t = NamedAtom;
using wow::http::client_t;
using wow::http::response_t;

TYPE_CONFIG (wow::http::client_t) {
    .type = "httpclient",
    .on_setup = [](state_t L) {
        using props_t = Properties<client_t>;
        using ref_t = client_t&;
        using cnst_t = const client_t&;
        props_t::add("host", [](state_t L, cnst_t self) {
            return lua::push(L, self.host());
        });
        props_t::add("isvalid", [](state_t L, cnst_t self) {
            return lua::push(L, self.is_valid());
        });
        props_t::add("port", [](state_t L, cnst_t self) {
            return lua::push(L, self.port());
        });
        props_t::add("encodeurl", nullptr, [](state_t L, ref_t self) {
            self.set_url_encode(luaL_checkboolean(L, 2));
        });
        props_t::add("keepalive", nullptr, [](state_t L, ref_t self) {
            self.set_keep_alive(luaL_checkboolean(L, 2));
        });
        props_t::add("connectiontimeout", nullptr, [](state_t L, ref_t self) {
            self.set_connection_timeout(std::chrono::milliseconds(luaL_checkinteger(L, 2)));
        });
        props_t::add("maxtimeout", nullptr, [](state_t L, ref_t self) {
            self.set_max_timeout(std::chrono::milliseconds(luaL_checkinteger(L, 2)));
        });
        props_t::add("readtimeout", nullptr, [](state_t L, ref_t self) {
            self.set_read_timeout(std::chrono::milliseconds(luaL_checkinteger(L, 2)));
        });
        props_t::add("writetimeout", nullptr, [](state_t L, ref_t self) {
            self.set_write_timeout(std::chrono::milliseconds(luaL_checkinteger(L, 2)));
        });
    },
    .namecall = [](lua_State* L) -> int {
        auto& self = Type<client_t>::to(L, 1);
        auto [atom, name] = lua::namecall_atom<NamedAtom>(L);
        switch (atom) {
            case NamedAtom::get: {
                auto r = self.Get(luaL_checkstring(L, 2));
                if (!r) return lua::push_tuple(L, lua::nil, std::format("error occurred ({})", static_cast<int>(r.error())));
                Type<response_t>::make(L, std::move(*r));
                return 1;
            }
            case named_t::stop:
                self.stop();
                return lua::none;
            default:
                luaL_errorL(L, "invalid namecall %s", name);
        }
        luaL_errorL(L, "invalid namecall %s", name);
    },
    .index = Properties<wow::http::client_t>::index,
    .newindex = Properties<wow::http::client_t>::newindex,
};


TYPE_CONFIG (response_t) {
    .type = "httpresponse",
    .on_setup = [](lua_State* L) {
        using props = Properties<response_t>;
        using cnst = const response_t&;
        props::add("body", [](state_t L, cnst self) {
            return lua::push(L, self.body);
        });
        props::add("status", [](state_t L, cnst self) {
            return lua::push(L, self.status);
        });
        props::add("location", [](state_t L, cnst self) {
            return lua::push(L, self.location);
        });
        props::add("version", [](state_t L, cnst self) {
            return lua::push(L, self.version);
        });
        props::add("reason", [](state_t L, cnst self) {
            return lua::push(L, self.reason);
        });
    },
    .namecall = [](lua_State* L) -> int {
        auto& self = Type<response_t>::to(L, 1);
        auto [atom, name] = lua::namecall_atom<NamedAtom>(L);
        switch (atom) {
            case named_t::getheaders:
                lua_newtable(L);
                for(const auto& [key, header] : self.headers) {
                    lua::set_field(L, key, header);
                }
                return 1;
            case named_t::getheadervalue: {
                auto def = luaL_optstring(L, 3, "");
                auto v = self.get_header_value(
                    luaL_checkstring(L, 2),
                    luaL_optstring(L, 3, "")
                );
                return lua::push(L, v);
            }
            default:
                luaL_errorL(L, "invalid namecall '%s'", name);
        }
    },
    .index = Properties<response_t>::index,
    .newindex = Properties<response_t>::newindex,
};
