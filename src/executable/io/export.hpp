#pragma once
#include <fstream>
#include <iostream>
#include <util/Interface.hpp>
struct lua_State;

namespace wow::io {
using writer_t = Interface<std::ostream>;
using reader_t = Interface<std::istream>;
using filewriter_t = std::ofstream;
using filereader_t = std::ifstream;
void library(lua_State* L, int idx);
}
