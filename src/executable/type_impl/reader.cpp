#include "common.hpp"
#include "named_atom.hpp"
#include "util/basic_type.hpp"
#include <bit>

using type = basic_type<reader>;
generate_type_trivial(reader, type);
template<class type>
static auto read(reader& from, lua_State* L, int idx = 2) -> int {
    auto arr = std::array<char, sizeof(type)>{};
    from->read(arr.data(), arr.size());
    return lua::push(L, static_cast<double>(std::bit_cast<type>(arr)));
};

static auto line_iterator_closure(lua_State* L) -> int {
    auto& self = as_reader(L, lua_upvalueindex(1));
    std::string line;
    if (std::getline(*self, line)) return lua::push(L, line);
    else return lua::none; 
}

auto test_namecall_reader(lua_State *L, reader& self, int atom) -> std::optional<int> {
    using named = named_atom;
    switch (static_cast<named>(atom)) {
        case named::readu8:
            return read<u8>(self, L);
        case named::readi8:
            return read<i8>(self, L);
        case named::readu16:
            return read<u16>(self, L);
        case named::readi16:
            return read<i16>(self, L);
        case named::readu32:
            return read<u32>(self, L);
        case named::readi32:
            return read<i32>(self, L);
        case named::readf32:
            return read<f32>(self, L);
        case named::readf64:
            return read<f64>(self, L);
        case named::line_iterator: {
            new_reader(L, {*self});
            lua_pushcclosure(L, line_iterator_closure, "line_iterator", 1);
            return 1;
        }
        default:
            return std::nullopt;
    }
}

