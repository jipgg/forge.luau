#include "common.hpp"
#include "utility/luau.hpp"
using builder_t = filewriter_builder_t;

static auto writer_namecall(state_t L) -> int {
    auto& self = builder_t::self(L);
    auto const [atom, name] = luau::namecall_atom<method_name>(L);
    switch (atom) {
        case method_name::write: {
            auto buf = luau::to_buffer(L, 2);
            self.write(&buf.front(), buf.size());
            return luau::none;
        }
        case method_name::write_string:
            self << luaL_checkstring(L, 2);
            return luau::none;
        case method_name::close:
            self.close();
            return luau::none;
        case method_name::is_open:
            return luau::push(L, self.is_open());
        case method_name::close_after: {
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
        case method_name::eof:
            return luau::push(L, self.eof());
        default:
            break;
    }
    luaL_errorL(L, "invalid namecall '%s'.", name);
}
static auto concat(state_t L) -> int {
    auto& self = builder_t::self(L);
    self << luau::tostring(L, 2);
    lua_pushvalue(L, 1);
    return 1;
}

void register_filewriter(state_t L) {
    luaL_Reg const metatable[] = {
        {"__namecall", writer_namecall},
        {"__concat", concat},
        {nullptr, nullptr}
    };
    builder_t::register_type(L, metatable);
}

