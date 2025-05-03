#include "common.hpp"
#include "named_atom.hpp"
#include "util/basic_type.hpp"
using type = basic_type<filewriter>;
generate_type_trivial(filewriter, type);

auto test_namecall_filewriter(lua_State* L, filewriter& self, int atom) -> std::optional<int> {
    switch (static_cast<named_atom>(atom)) {
        // case named_atom::close:
        //     self.close();
        //     return lua::none;
        case named_atom::isopen:
            return lua::push(L, self.is_open());
        case named_atom::close: {
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
        default: {
            auto ref = writer{self};
            return test_namecall_writer(L, ref, atom);
        }
    }
}
static auto namecall(lua_State* L) -> int {
    auto& self = type::get(L, 1);
    auto const [atom, name] = lua::namecall_atom(L);
    auto pushed = test_namecall_filewriter(L, self, atom);
    if (not pushed) luaL_errorL(L, "invalid namecall '%s'.", name);
    return *pushed;
}
static auto call(lua_State* L) -> int {
    auto& self = type::get(L, 1);
    self << lua::tostring_tuple(L, {.start_index = 2, .separator = ", "});
    lua_pushvalue(L, 1);
    return 1;
}
void load_filewriter(lua_State* L) {
    luaL_Reg const metatable[] = {
        {"__namecall", namecall},
        {"__call", call},
        {nullptr, nullptr}
    };
    type::setup(L, metatable);
}
