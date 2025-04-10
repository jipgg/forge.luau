#include "common.hpp"
#include <filesystem>
#include <fstream>
namespace fs = std::filesystem;

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
// struct file_writer: public writer_t {
//     std::ofstream file;
//     fs::path path;
//     bool append = false;
//     auto open() -> std::ostream& override {
//         file.open(path);
//         return file;
//     }
//     void close() override {
//         file.close();
//     }
//     auto get() -> std::ostream& override {
//         return file;
//     }
// };
// static auto writer(state_t L) -> int {
//     push_writer(L, file_writer{.path = to_path(L, 2)});
// }

void open_fileio(state_t L, library_config config) {
    const luaL_Reg fileio[] = {
        {"write_file", write_file},
        {"read_file", read_file},
        {nullptr, nullptr}
    };
    config.apply(L, fileio);
}
