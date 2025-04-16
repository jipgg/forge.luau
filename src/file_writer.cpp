#include "common.hpp"
#include "utility/luau.hpp"
using builder_t = file_writer_builder_t;

static auto writer_namecall(state_t L) -> int {
    auto& self = builder_t::self(L);
    auto const [atom, name] = luau::namecall_atom<method_name>(L);
    switch (atom) {
        case method_name::write:
            self << luaL_checkstring(L, 2);
            return luau::none;
        case method_name::close:
            self.close();
            return luau::none;
        default:
            break;
    }
    luaL_errorL(L, "invalid namecall '%s'.", name);
}
static auto add(state_t L) -> int {
    auto& self = builder_t::self(L);
    self << luau::tostring(L, 2);
    lua_pushvalue(L, 1);
    return 1;
}

void register_file_writer(state_t L) {
    luaL_Reg const metatable[] = {
        {"__namecall", writer_namecall},
        {"__add", add},
        {nullptr, nullptr}
    };
    builder_t::register_type(L, metatable);
}

