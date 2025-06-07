#include "export.hpp"
#include "named_atom.hpp"
#include "lua/lua.hpp"
#include "lua/typeutility.hpp"
using state = lua_State*;
using self = lib::http::client;
using props = lua::properties<self>;
using type = lua::type<self>;
using response_type = lua::type<lib::http::response>;

TYPE_CONFIG (lib::http::client) {
    .type = "httpclient",
    .on_setup = [](state L) {
        props::add("host", [](state L, const self& self) {
            return lua::push(L, self.host());
        });
        props::add("isvalid", [](state L, const self& self) {
            return lua::push(L, self.is_valid());
        });
        props::add("port", [](state L, const self& self) {
            return lua::push(L, self.port());
        });
        props::add("encodeurl", nullptr, [](state L, self& self) {
            self.set_url_encode(luaL_checkboolean(L, 2));
        });
        props::add("keepalive", nullptr, [](state L, self& self) {
            self.set_keep_alive(luaL_checkboolean(L, 2));
        });
        props::add("connectiontimeout", nullptr, [](state L, self& self) {
            self.set_connection_timeout(std::chrono::milliseconds(luaL_checkinteger(L, 2)));
        });
        props::add("maxtimeout", nullptr, [](state L, self& self) {
            self.set_max_timeout(std::chrono::milliseconds(luaL_checkinteger(L, 2)));
        });
        props::add("readtimeout", nullptr, [](state L, self& self) {
            self.set_read_timeout(std::chrono::milliseconds(luaL_checkinteger(L, 2)));
        });
        props::add("writetimeout", nullptr, [](state L, self& self) {
            self.set_write_timeout(std::chrono::milliseconds(luaL_checkinteger(L, 2)));
        });
    },
    .namecall = [](state L) -> int {
        auto& self = type::to(L, 1);
        auto [atom, name] = lua::namecall_atom<named_atom>(L);
        switch (atom) {
            case named_atom::get: {
                auto r = self.Get(luaL_checkstring(L, 2));
                if (!r) return lua::push_tuple(L, lua::nil, std::format("error occurred ({})", static_cast<int>(r.error())));
                response_type::make(L, std::move(*r));
                return 1;
            }
            case named_atom::stop:
                self.stop();
                return lua::none;
            default:
                luaL_errorL(L, "invalid namecall %s", name);
        }
        luaL_errorL(L, "invalid namecall %s", name);
    },
    .index = props::index,
    .newindex = props::newindex,
};
