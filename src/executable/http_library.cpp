#include "common.hpp"
#include <httplib.h>
#include <httplib.h>

static auto client(lua_State* L) -> int {
    Type<HttpClient>::make(L, luaL_checkstring(L, 1));
    return 1;
}
void loader::http(lua_State* L, int idx) {
    lua::set_functions(L, idx, std::to_array<luaL_Reg>({
        {"client", client},
    }));
}
// void open_httplib(lua_State* L) {
//     luaL_Reg functions[] = {
//         {"client", client},
//         {nullptr, nullptr}
//     };
//     luaL_register(L, "http", functions);
// }
