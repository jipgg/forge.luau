// #include "builtin.hpp"
// #include "userdata.hpp"
// #include "lua.hpp"
// register_type_name(writer_t, "writer");
//
//
// static auto writer_namecall(state_t L) -> int {
//     auto& self = userdata::self<writer_t>(L);
//     self.open();
//     defer close{[&] {self.close();}};
//
//     auto const [atom, name] = lua::namecall_atom<Method>(L);
//     switch (atom) {
//         case Method::write:
//             *self << luaL_checkstring(L, 2);
//             return lua::none;
//         case Method::writeln:
//             *self << luaL_checkstring(L, 2) << '\n';
//             return lua::none;
//         case Method::print:
//             *self << lua::tostring_tuple(L, {.start_index = 2}) << '\n';
//             return lua::none;
//         default:
//             break;
//     }
//     luaL_errorL(L, "invalid namecall '%s'.", name);
// }
//
// void register_writer(state_t L) {
//     auto name = userdata::name_for<writer_t>();
//     if (luaL_newmetatable(L, name)) {
//         const luaL_Reg metatable[] = {
//             {"__namecall", writer_namecall},
//             {nullptr, nullptr}
//         };
//         luaL_register(L, nullptr, metatable);
//         lua::push(L, name);
//         lua_setfield(L, -2, "__type");
//     }
//     lua_pop(L, 1);
//     userdata::set_dtor<writer_t>(L);
// }
//
// auto push_writer(state_t L, const writer_t &writer) -> int {
//     return userdata::push(L, writer);
// }
