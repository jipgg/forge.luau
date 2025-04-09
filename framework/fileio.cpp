#include "framework.hpp"
#include <filesystem>
#include <fstream>
namespace fs = std::filesystem;

static auto write_file(state_t L) -> int {
    fs::path destination = fw::check_path(L, 1);
    std::string contents = luaL_checkstring(L, 2);
    std::ofstream file{destination};
    file << contents;
    return 0;
}
static auto read_file(state_t L) -> int {
    auto path = fw::check_path(L, 1);
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    if (!file) luaL_errorL(L, "failed to open file '%s'.", path.string().c_str());

    // Get the file size
    auto size = file.tellg();
    file.seekg(0, std::ios::beg);

    auto content = std::string(size, '\0');

    if (!file.read(&content[0], size)) {
        luaL_errorL(L, "failed to read file '%s'.", path.string().c_str());
    }
    lua_pushstring(L, content.c_str());
    return 1;
}

void fileio::get(state_t L, const char* name) {
    const luaL_Reg fileio[] = {
        {"write_file", write_file},
        {"read_file", read_file},
        {nullptr, nullptr}
    };
    lua_newtable(L);
    luaL_register(L, nullptr, fileio);
    if (name) lua_setfield(L, -2, name);
}
