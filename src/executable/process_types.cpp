#include "common.hpp"
#include "NamedAtom.hpp"
#include "util/lua.hpp"
#include "util/type_utility.hpp"

// TYPE_CONFIG (Process) {
//     .type = "subprocess",
//     .namecall = [](lua_State* L) -> int {
//         auto& self = Type<Process>::to(L, 1);
//         auto const [atom, name] = lua::namecall_atom<NamedAtom>(L);
//         switch (atom) {
//             default:
//                 luaL_errorL(L, "invalid namecall");
//         }
//     },
// };
