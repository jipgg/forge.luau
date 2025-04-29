#include "common.hpp"
#include "named_atom.hpp"
#include "util/basic_type.hpp"
#include <bit>

using type = basic_type<writer>;
generate_type_trivial(writer, type);

template<class type>
static auto write_number_raw(writer& to, lua_State* L, int idx = 2) -> size_t {
    auto data = static_cast<type>(luaL_checknumber(L, idx));
    auto bytes = std::bit_cast<std::array<uint8_t, sizeof(type)>>(data);
    for (auto byte : bytes) to->put(byte);
    return sizeof(type);
};
auto test_namecall_writer(lua_State* L, writer& self, int atom) -> std::optional<int> {
    auto push_self = [&] {
        lua_pushvalue(L, 1);
        return 1;
    };
    using named = named_atom;
    switch (static_cast<named>(atom)) {
        case named::writestring:
            *self << luaL_checkstring(L, 2);
            return push_self();
        case named::writeu8:
            write_number_raw<uint8_t>(self, L);
            return push_self();
        case named::writei8:
            write_number_raw<int8_t>(self, L);
            return push_self();
        case named::writeu16:
            write_number_raw<uint16_t>(self, L);
            return push_self();
        case named::writei16:
            write_number_raw<int16_t>(self, L);
            return push_self();
        case named::writeu32:
            write_number_raw<uint32_t>(self, L);
            return push_self();
        case named::writei32:
            write_number_raw<int32_t>(self, L);
            return push_self();
        case named::writef32:
            write_number_raw<float>(self, L);
            return push_self();
        case named::writef64:
            write_number_raw<double>(self, L);
            return push_self();
        case named::write: {
            auto buf = lua::to_buffer(L, 2);
            self->write(&buf.front(), buf.size());
            return push_self();
        }
        case named::print:
            *self << lua::tostring_tuple(L, {.start_index = 2, .separator = "\t"});
            *self << luaL_checkstring(L, 2);
            return push_self();
        case named::writeln: {
            auto buf = lua::to_buffer(L, 2);
            self->write(&buf.front(), buf.size());
            self->put('\n');
            return push_self();
        }
        case named::println:
            *self << lua::tostring_tuple(L, {
                .start_index = 2,
                .separator = "\t"}) << '\n';
            return push_self();
        case named::ln:
            self->put('\n');
            return push_self();
        case named::endl:
            *self << std::endl;
            return push_self();
        case named::flush:
            self->flush();
            return push_self();
        case named::eof:
            return lua::push(L, self->eof());
        default: return std::nullopt;
    }
}
static auto namecall(lua_State* L) -> int {
    auto& self = type::get(L, 1);
    auto const [atom, name] = lua::namecall_atom(L);
    auto pushed = test_namecall_writer(L, self, atom);
    if (not pushed) luaL_errorL(L, "invalid namecall '%s'.", name);
    return *pushed;
}
void register_writer(lua_State* L) {
    luaL_Reg const metatable[] = {
        {"__namecall", namecall},
        {nullptr, nullptr}
    };
    type::setup(L, metatable);
}
