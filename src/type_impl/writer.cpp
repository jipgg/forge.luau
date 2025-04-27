#include "common.hpp"
#include "utility/luau.hpp"
#include "method.hpp"
using util = writer::util;
template<>
const char* util::name{"plus_writer"};

template<>
auto writer::name() -> const char* {return util::name;}
template<>
auto writer::namecall_if(state_t L, writer_t& self, int atom) -> std::optional<int> {
    auto push_self = [&] {
        lua_pushvalue(L, 1);
        return 1;
    };
    switch (static_cast<method>(atom)) {
        case method::write: {
            auto buf = luau::to_buffer(L, 2);
            self->write(&buf.front(), buf.size());
            return push_self();
        }
        case method::print:
            *self << luaL_checkstring(L, 2);
            return push_self();
        case method::writeln: {
            auto buf = luau::to_buffer(L, 2);
            self->write(&buf.front(), buf.size());
            self->put('\n');
            return push_self();
        }
        case method::println:
            *self << luaL_checkstring(L, 2) << '\n';
            return push_self();
        case method::ln:
            self->put('\n');
            return push_self();
        case method::endl:
            *self << std::endl;
            return push_self();
        case method::flush:
            self->flush();
            return push_self();
        case method::eof:
            return luau::push(L, self->eof());
        default: return std::nullopt;
    }
}
static auto mt_namecall(state_t L) -> int {
    auto& self = util::self(L);
    auto const [atom, name] = luau::namecall_atom(L);
    auto pushed = writer::namecall_if(L, self, atom);
    if (not pushed) luaL_errorL(L, "invalid namecall '%s'.", name);
    return *pushed;
}
static auto mt_concat(state_t L) -> int {
    auto& self = util::self(L);
    *self << luau::tostring(L, 2);
    lua_pushvalue(L, 1);
    return 1;
}

template<>
void writer::init(state_t L) {
    luaL_Reg const metatable[] = {
        {"__namecall", mt_namecall},
        {"__concat", mt_concat},
        {nullptr, nullptr}
    };
    util::register_type(L, metatable);
}
