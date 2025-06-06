#pragma once
struct lua_State;

namespace wow::proc {
void library(lua_State* L, int idx);
}
