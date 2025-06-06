#include "export.hpp"
#include <nlohmann/json.hpp>
#include "util/lua.hpp"
#include "util/json_utility.hpp"

static auto parse(lua_State* L) -> int {
    try {
        auto parsed = nlohmann::json::parse(luaL_checkstring(L, 1));
        util::push_value(L, parsed);
        return 1;
    } catch (nlohmann::json::exception& e) {
        luaL_errorL(L, "%s", e.what());
    }
}
static auto tostring(lua_State* L) -> int {
    return lua::push(L, util::table_to_json(L, 1).dump());
}
void wow::json::library(lua_State* L, int idx) {
    lua::set_functions(L, idx, std::to_array<luaL_Reg>({
        {"tostring", tostring},
        {"parse", parse},
    }));
}
