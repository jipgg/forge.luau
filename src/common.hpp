#ifndef COMMON_HPP
#define COMMON_HPP

#include <memory>
#include <expected>
#include <filesystem>
#include <lualib.h>
#include "utility/compile_time.hpp"
#include <fstream>
#include "utility/luau.hpp"
#include <cassert>
using state_t = lua_State*;
using state_owner_t = std::unique_ptr<lua_State, decltype(&lua_close)>;
using library_loader_t = std::function<void(state_t L)>;

struct args_wrapper {
    std::span<char*> data;
    auto view() const -> decltype(auto) {
        return std::views::transform(data, [](auto v) -> std::string_view {return v;});
    }
    auto operator[](size_t idx) const -> std::optional<std::string_view> {
        if (idx >= data.size()) return std::nullopt;
        return data[idx];
    }
    args_wrapper(int argc, char** argv): data(argv, argv + argc) {}
    explicit args_wrapper(std::span<char*> data): data(data) {}
    explicit args_wrapper(): data() {}
};

struct library_config {
    const char* name = nullptr;
    bool local = false;
    void apply(state_t L, library_loader_t loader) {
        if (local) {
            lua_newtable(L);
            loader(L);
            if (name) lua_setfield(L, -2, name);
            return;
        } else if (name) {
            lua_newtable(L);
            loader(L);
            lua_setglobal(L, name);
            return;
        } else {
            lua_pushvalue(L, LUA_GLOBALSINDEX);
            loader(L);
            lua_pop(L, 1);
            return;
        }
    }
    void apply(state_t L, const luaL_Reg* lib) {
        apply(L, [&lib](auto L) {
            luaL_register(L, nullptr, lib);
        });
    }
};
void open_global(state_t L, const library_config& config = {.name = nullptr});
void open_filesystem(state_t L, library_config config = {.name = "filesystem"});
void open_fileio(state_t L, library_config config = {.name = "fileio"});
void open_json(state_t L, library_config config = {.name = "json"});
void open_process(state_t L, library_config config = {.name = "process"});
auto get_args() -> args_wrapper const&;

auto setup_state() -> state_owner_t;
auto load_script(state_t L, const std::filesystem::path& path) -> std::expected<state_t, std::string>;

using path_t = std::filesystem::path;
using path_builder_t = luau::generic_userdatatagged_builder<path_t>;
template<> inline const char* path_builder_t::name{"path"};
void register_path(state_t L);
auto to_path(state_t L, int idx) -> path_t;

using filewriter_t = std::ofstream;
using filewriter_builder_t = luau::generic_userdatatagged_builder<filewriter_t>;
template<> inline const char* filewriter_builder_t::name{"filewriter"};
void register_filewriter(state_t L);

enum class method_name {
    read_all,
    children,
    string,
    generic_string,
    extension,
    has_extension,
    replace_extension,
    has_stem,
    stem,
    filename,
    has_filename,
    replace_filename,
    remove_filename,
    parent_path,
    has_parent_path,
    relative_path,
    has_relative_path,
    is_absolute,
    is_relative,
    root_directory,
    has_root_directory,
    lexically_normal,
    lexically_proximate,
    lexically_relative,
    print,
    write,
    writeln,
    flush,
    is_open,
    scope,
    line_iterator,
    eof,
    close,
    close_after,
    COMPILE_TIME_ENUM_SENTINEL
};

#endif
