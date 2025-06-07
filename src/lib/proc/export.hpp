#pragma once
struct lua_State;

namespace lib::proc {
void library(lua_State* L, int idx);
}
