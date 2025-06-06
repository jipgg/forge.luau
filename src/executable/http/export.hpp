#pragma once
#include <httplib.h>
struct lua_State;

namespace wow::http {
using client_t = httplib::Client;
using response_t = httplib::Response;
void library(lua_State* L, int idx);
}
