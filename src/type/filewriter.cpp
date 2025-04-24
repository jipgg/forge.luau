#include "common.hpp"
#include "utility/luau.hpp"
#include "method.hpp"
using util = type::filewriter::util;
template<>
const char* util::name{"filewriter"};

namespace type {
template<>
auto filewriter::name() -> const char* {return util::name;}
template<>
auto filewriter::namecall_if(state_t L, filewriter::type& self, int atom) -> std::optional<int> {
    switch (static_cast<method>(atom)) {
        case method::write: {
            auto buf = luau::to_buffer(L, 2);
            self.write(&buf.front(), buf.size());
            return luau::none;
        }
        case method::write_string:
            self << luaL_checkstring(L, 2);
            return luau::none;
        case method::close:
            self.close();
            return luau::none;
        case method::is_open:
            return luau::push(L, self.is_open());
        case method::close_after: {
            lua_pushvalue(L, 2);
            lua_pushvalue(L, 1);
            auto return_values = *luau::pcall(L, 1).transform([&] {
                return luau::push(L, true);
            }).transform_error([&] (auto const& e) {
                return luau::push_values(L, luau::nil, e);
            });
            self.close();
            return return_values;
        }
        case method::eof:
            return luau::push(L, self.eof());
        default: return std::nullopt;
    }

}
static auto mt_namecall(state_t L) -> int {
    auto& self = util::self(L);
    auto const [atom, name] = luau::namecall_atom(L);
    auto pushed = filewriter::namecall_if(L, self, atom);
    if (not pushed) luaL_errorL(L, "invalid namecall '%s'.", name);
    return *pushed;
}
static auto mt_concat(state_t L) -> int {
    auto& self = util::self(L);
    self << luau::tostring(L, 2);
    lua_pushvalue(L, 1);
    return 1;
}

template<>
void filewriter::init(state_t L) {
    luaL_Reg const metatable[] = {
        {"__namecall", mt_namecall},
        {"__concat", mt_concat},
        {nullptr, nullptr}
    };
    util::register_type(L, metatable);
}
}
