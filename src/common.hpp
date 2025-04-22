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
void open_consoleio(state_t L, library_config config = {.name = "consoleio"});
void open_json(state_t L, library_config config = {.name = "json"});
void open_process(state_t L, library_config config = {.name = "process"});
auto get_args() -> args_wrapper const&;

auto setup_state() -> state_owner_t;
auto load_script(state_t L, const std::filesystem::path& path) -> std::expected<state_t, std::string>;

using path_t = std::filesystem::path;
using path_builder_t = luau::generic_userdatatagged_builder<path_t>;
template<> inline const char* path_builder_t::name{"__path"};
void register_path(state_t L);
auto to_path(state_t L, int idx) -> path_t;

using filewriter_t = std::ofstream;
using filewriter_builder_t = luau::generic_userdatatagged_builder<filewriter_t>;
template<> inline const char* filewriter_builder_t::name{"__filewriter"};
void register_filewriter(state_t L);

namespace detail {
template <class Iterator>
inline auto directory_iterator_closure(state_t L) -> int {
    auto& it = luau::to_userdata<Iterator>(L, lua_upvalueindex(1));
    const Iterator end{};
    if (it != end) {
        const std::filesystem::directory_entry& entry = *it;
        auto path = entry.path();
        path_builder_t::push(L, path);
        ++it;
        return 1;
    }
    return 0;
}
}
inline auto push_directory_iterator(state_t L, path_t const& directory, bool recursive) -> int {
    namespace fs = std::filesystem;
    if (not fs::is_directory(directory)) {
        luaL_errorL(L, "path '%s' must be a directory", directory.string().c_str());
    }
    if (not recursive) {
        luau::make_userdata<fs::directory_iterator>(L, directory);
        lua_pushcclosure(L,
            detail::directory_iterator_closure<fs::directory_iterator>,
            "directory_iterator",
            1
        );
    } else {
        luau::make_userdata<fs::recursive_directory_iterator>(L, directory);
        lua_pushcclosure(
            L,
            detail::directory_iterator_closure<fs::recursive_directory_iterator>,
            "recursive_directory_iterator",
            1
        );
    }
    return 1;
}

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
    write_string,
    absolute,
    is_directory,
    is_file,
    is_symlink,
    directory_iterator,
    COMPILE_TIME_ENUM_SENTINEL
};

#endif
