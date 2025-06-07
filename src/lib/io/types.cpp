#include "export.hpp"
#include "named_atom.hpp"
#include "lua/lua.hpp"
#include "lua/typeutility.hpp"
#include <bit>
using lib::io::reader;
using lib::io::writer;
using lib::io::filewriter;
using lib::io::filereader;

template<typename T>
static auto read(reader& from, lua_State* L, int idx = 2) -> int {
    auto arr = std::array<char, sizeof(T)>{};
    from->read(arr.data(), arr.size());
    return lua::push(L, static_cast<double>(std::bit_cast<T>(arr)));
};
static auto line_iterator_closure(lua_State* L) -> int {
    auto& self = lua::type<reader>::to(L, lua_upvalueindex(1));
    std::string line;
    if (std::getline(*self, line)) return lua::push(L, line);
    else return lua::none; 
}
template<typename U>
static auto write_number_raw(writer& to, lua_State* L, int idx = 2) -> size_t {
    auto data = static_cast<U>(luaL_checknumber(L, idx));
    auto bytes = std::bit_cast<std::array<uint8_t, sizeof(U)>>(data);
    for (auto byte : bytes) to->put(byte);
    return sizeof(U);
};
static auto reader_namecall(lua_State *L, reader& self, named_atom atom) -> std::optional<int> {
    using named = named_atom;
    switch (static_cast<named>(atom)) {
        case named::readu8: return read<uint8_t>(self, L);
        case named::readi8: return read<int8_t>(self, L);
        case named::readu16: return read<uint16_t>(self, L);
        case named::readi16: return read<int16_t>(self, L);
        case named::readu32: return read<uint32_t>(self, L);
        case named::readi32: return read<int32_t>(self, L);
        case named::readf32: return read<float>(self, L);
        case named::readf64: return read<double>(self, L);
        case named::scan: {
            std::string str{};
            *self >> str; 
            return lua::push(L, str);
        }
        case named::lines: {
            lua::type<reader>::make(L, reader{*self});
            lua_pushcclosure(L, line_iterator_closure, "line_iterator", 1);
            return 1;
        }
        default:
            return std::nullopt;
    }
}
auto writer_namecall(lua_State* L, writer& self, named_atom atom) -> std::optional<int> {
    auto& os = *self;
    auto push_self = [&] {
        lua_pushvalue(L, 1);
        return 1;
    };
    using Named = named_atom;
    switch (static_cast<Named>(atom)) {
        case Named::writeu8:
            write_number_raw<uint8_t>(self, L);
            return push_self();
        case Named::writei8:
            write_number_raw<int8_t>(self, L);
            return push_self();
        case Named::writeu16:
            write_number_raw<uint16_t>(self, L);
            return push_self();
        case Named::writei16:
            write_number_raw<int16_t>(self, L);
            return push_self();
        case Named::writeu32:
            write_number_raw<uint32_t>(self, L);
            return push_self();
        case Named::writei32:
            write_number_raw<int32_t>(self, L);
            return push_self();
        case Named::writef32:
            write_number_raw<float>(self, L);
            return push_self();
        case Named::writef64:
            write_number_raw<double>(self, L);
            return push_self();
        case Named::write: {
            if (lua_isbuffer(L, 2)) {
                auto buf = lua::to_buffer(L, 2);
                os.write(&buf.front(), buf.size());
            } else if (lua_isstring(L, 2)) {
                os << luaL_checkstring(L, 2);
                return push_self();
            }
            break;
        }
        case Named::flush:
            os.flush();
            return push_self();
        case Named::eof:
            return lua::push(L, self->eof());
        case Named::good:
            return lua::push(L, self->good());
        case Named::bad:
            return lua::push(L, self->bad());
        case Named::fail:
            return lua::push(L, self->fail());
        case Named::clear:
            self->clear();
            return push_self();
        case Named::seek:
            self->seekp(luaL_checkinteger(L, 2), std::ios::beg);
            return push_self();
        case Named::tell:
            return lua::push(L, static_cast<int>(self->tellp()));
        default: return std::nullopt;
    }
    return std::nullopt;
}
TYPE_CONFIG (writer) {
    .type = "writer",
    .namecall = [](lua_State* L) -> int {
        auto& self = lua::type<writer>::to(L, 1);
        auto const [atom, name] = lua::namecall_atom<named_atom>(L);
        auto pushed = writer_namecall(L, self, atom);
        if (not pushed) luaL_errorL(L, "invalid namecall '%s'.", name);
        return *pushed;
    },
    .call = [](lua_State* L) {
        auto& self = *lua::type<writer>::to(L, 1).get();
        self << lua::tostring_tuple(L, {.start_index = 2, .separator = ", "});
        lua_pushvalue(L, 1);
        return 1;
    },
};
TYPE_CONFIG (lib::io::filewriter) {
    .type = "filewriter",
    .on_setup = [](lua_State* L) {
        lua::properties<lib::io::filewriter>::add("isopen", [](auto L, auto const& self) {
            return lua::push(L, self.is_open());
        });
    },
    .namecall = [](lua_State* L) -> int {
        auto& self = lua::type<filewriter>::to(L, 1);
        auto const [atom, name] = lua::namecall_atom<named_atom>(L);
        if (atom == named_atom::close) {
            if (lua_isnoneornil(L, 2)) {
                self.close();
                return lua::none;
            } else {
                lua_pushvalue(L, 2);
                lua_pushvalue(L, 1);
                auto return_values = *lua::pcall(L, 1).transform([&] {
                    return lua::push(L, true);
                }).transform_error([&] (auto const& e) {
                    return lua::push_tuple(L, lua::nil, e);
                });
                self.close();
                return return_values;
            }
        }
        auto ref = writer{self};
        auto pushed = writer_namecall(L, ref, atom);
        if (not pushed) luaL_errorL(L, "invalid namecall '%s'.", name);
        return *pushed;
    },
    .call = [](auto L) {
        auto& self = lua::type<lib::io::filewriter>::to(L, 1);
        self << lua::tostring_tuple(L, {.start_index = 2, .separator = ", "});
        lua_pushvalue(L, 1);
        return 1;
    },
    .index = lua::properties<filewriter>::index,
    .newindex = lua::properties<filewriter>::newindex,
};
TYPE_CONFIG (filereader) {
    .type = "filereader",
    .namecall = [](lua_State* L) -> int {
        auto& self = lua::type<filereader>::to(L, 1);
        auto const [atom, name] = lua::namecall_atom<named_atom>(L);
        if (atom == named_atom::close) {
            if (lua_isnoneornil(L, 2)) {
                self.close();
                return lua::none;
            } else {
                lua_pushvalue(L, 2);
                lua_pushvalue(L, 1);
                auto return_values = *lua::pcall(L, 1).transform([&] {
                    return lua::push(L, true);
                }).transform_error([&] (auto const& e) {
                    return lua::push_tuple(L, lua::nil, e);
                });
                self.close();
                return return_values;
            }
        }
        auto ref = reader{self};
        auto pushed = reader_namecall(L, ref, atom);
        if (not pushed) luaL_errorL(L, "invalid namecall '%s'.", name);
        return *pushed;
    }
};
TYPE_CONFIG (reader) {
    .type = "reader",
    .namecall = [](lua_State* L) -> int {
        auto& self = lua::type<reader>::to(L, 1);
        auto const [atom, name] = lua::namecall_atom<named_atom>(L);
        auto pushed = reader_namecall(L, self, atom);
        if (not pushed) luaL_errorL(L, "invalid namecall '%s'.", name);
        return *pushed;
    },
};
