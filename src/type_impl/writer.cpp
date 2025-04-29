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
        case named::write_string:
            *self << luaL_checkstring(L, 2);
            return push_self();
        case named::write_u8:
            write_number_raw<uint8_t>(self, L);
            return push_self();
        case named::write_i8:
            write_number_raw<int8_t>(self, L);
            return push_self();
        case named::write_u16:
            write_number_raw<uint16_t>(self, L);
            return push_self();
        case named::write_i16:
            write_number_raw<int16_t>(self, L);
            return push_self();
        case named::write_u32:
            write_number_raw<uint32_t>(self, L);
            return push_self();
        case named::write_i32:
            write_number_raw<int32_t>(self, L);
            return push_self();
        case named::write_f32:
            write_number_raw<float>(self, L);
            return push_self();
        case named::write_f64:
            write_number_raw<double>(self, L);
            return push_self();
        case named::write_buffer: {
            auto buf = lua::to_buffer(L, 2);
            self->write(&buf.front(), buf.size());
            return push_self();
        }
        case named::print:
            *self << lua::tostring_tuple(L, {.start_index = 2, .separator = "\t"});
            *self << luaL_checkstring(L, 2);
            return push_self();
        case named::println:
            *self << lua::tostring_tuple(L, {
                .start_index = 2,
                .separator = ", "}) << '\n';
            return push_self();
        case named::ln:
            self->put('\n');
            return push_self();
        case named::tab:
            self->put('\t');
            return push_self();
        case named::endl:
            *self << std::endl;
            return push_self();
        case named::flush:
            self->flush();
            return push_self();
        case named::eof:
            return lua::push(L, self->eof());
        case named::good:
            return lua::push(L, self->good());
        case named::bad:
            return lua::push(L, self->bad());
        case named::fail:
            return lua::push(L, self->fail());
        case named::clear:
            self->clear();
            return push_self();
        case named::seekp:
            self->seekp(luaL_checkinteger(L, 2));
            return push_self();
        case named::tellp:
            return lua::push(L, static_cast<int>(self->tellp()));
        case named::esc:
            self->put('\033');
            return push_self();
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
