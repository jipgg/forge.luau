#include "common.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
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

static auto write_string(state_t L) -> int {
    fs::path destination = to_path(L, 1);
    std::string contents = luaL_checkstring(L, 2);
    std::ofstream file{destination};
    file << contents;
    return 0;
}
static auto write(state_t L) -> int {
    fs::path destination = to_path(L, 1);
    auto buf = luau::to_buffer(L, 2);
    auto contents = std::string{buf.data(), buf.size()};
    std::ofstream file{destination};
    file << contents;
    return 0;
}
static auto read_to_end(state_t L) -> int {
    auto path = to_path(L, 1);
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    if (!file) luaL_errorL(L, "failed to open file '%s'.", path.string().c_str());

    // Get the file size
    auto size = file.tellg();
    file.seekg(0, std::ios::beg);

    //auto content = std::string(size, '\0');
    auto content = luau::make_buffer(L, size);

    if (!file.read(&content[0], size)) {
        luaL_errorL(L, "failed to read file '%s'.", path.string().c_str());
    }
    //luau::make_buffer(L, content);
    return 1;
}
static auto open_writer(state_t L) -> int {
    auto path = to_path(L, 1);
    auto append_mode = luaL_optboolean(L, 2, false);
    auto file = std::ofstream{};
    if (append_mode) {
        check_open(L, path, filewriter::util::make(L, path, std::ios::app));
    } else {
        check_open(L, path, filewriter::util::make(L, path));
    }
    return 1;
}
static auto append(state_t L) -> int {
    auto path = to_path(L, 1);
    auto file = std::ofstream{path, std::ios::app};
    check_open(L, path, file);
    file << luaL_checkstring(L, 2);
    return luau::none;
}
static auto append_line(state_t L) -> int {
    auto path = to_path(L, 1);
    auto file = std::ofstream{path, std::ios::app};
    check_open(L, path, file);
    file << luaL_checkstring(L, 2) << std::endl;
    return luau::none;
}
template<>
void fileio::open(state_t L, library_config config) {
    const luaL_Reg fileio[] = {
        {"write_string", write_string},
        {"write", write},
        {"read_to_end", read_to_end},
        {"open_writer", open_writer},
        {"append_line", append_line},
        {"append", append},
        {nullptr, nullptr}
    };
    config.apply(L, fileio);
}
