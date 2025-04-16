#include "common.hpp"
#include <filesystem>
#include <fstream>
namespace fs = std::filesystem;

template<class Ty>
concept has_is_open = requires (Ty& v) {
{v.is_open()} -> std::same_as<bool>;
};

template<has_is_open Ty>
static auto check_open(state_t L, fs::path const& path, Ty& file) -> void {
    if (not file.is_open()) {
        luaL_errorL(L, "failed to open file '%s'.", path.string().c_str());
    }
}

static auto write_file(state_t L) -> int {
    fs::path destination = to_path(L, 1);
    std::string contents = luaL_checkstring(L, 2);
    std::ofstream file{destination};
    file << contents;
    return 0;
}
static auto read_file(state_t L) -> int {
    auto path = to_path(L, 1);
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
static auto open_writer(state_t L) -> int {
    auto path = to_path(L, 1);
    auto& file = file_writer_builder_t::make(L, path);
    check_open(L, path, file);
    return 1;
}
static auto open_append_writer(state_t L) -> int {
    auto path = to_path(L, 1);
    auto& file = file_writer_builder_t::make(L, path, std::ios::app);
    check_open(L, path, file);
    return 1;
}
void open_fileio(state_t L, library_config config) {
    const luaL_Reg fileio[] = {
        {"write_all", write_file},
        {"read_all", read_file},
        {"open_writer", open_writer},
        {"open_append_writer", open_append_writer},
        {nullptr, nullptr}
    };
    config.apply(L, fileio);
}
