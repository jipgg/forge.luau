#pragma once
#include <httplib.h>
struct lua_State;

namespace lib::http {
using client = httplib::Client;
using response = httplib::Response;
void library(lua_State* L, int idx);
}
