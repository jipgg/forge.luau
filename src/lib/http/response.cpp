#include "export.hpp"
#include "named_atom.hpp"
#include "lua/lua.hpp"
#include "lua/typeutility.hpp"
using state = lua_State*;
using named_t = named_atom;
using self = lib::http::response;
using props = lua::properties<self>;
using type = lua::type<self>;

TYPE_CONFIG (lib::http::response) {
    .type = "httpresponse",
    .on_setup = [](lua_State* L) {
        props::add("body", [](state L, const self& self) {
            return lua::push(L, self.body);
        });
        props::add("status", [](state L, const self& self) {
            return lua::push(L, self.status);
        });
        props::add("location", [](state L, const self& self) {
            return lua::push(L, self.location);
        });
        props::add("version", [](state L, const self& self) {
            return lua::push(L, self.version);
        });
        props::add("reason", [](state L, const self& self) {
            return lua::push(L, self.reason);
        });
    },
    .namecall = [](lua_State* L) -> int {
        auto& self = type::to(L, 1);
        auto [atom, name] = lua::namecall_atom<named_atom>(L);
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
    .index = props::index,
    .newindex = props::newindex,
};
