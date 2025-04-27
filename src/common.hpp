#ifndef COMMON_HPP
#define COMMON_HPP
#include <memory>
#include <expected>
#include <filesystem>
#include <lualib.h>
#include <Luau/Require.h>
#include "utility/compile_time.hpp"
#include "utility/luau.hpp"
#include "utility/library_config.hpp"
#include <fstream>
#include <expected>
#include <cassert>
#include <variant>
using luau::state_t;
using luau::state_owner_t;

template<class Ty>
struct type_impl {
    using type = Ty;
    using util = luau::generic_userdatatagged_builder<Ty>;
    static void init(state_t L);
    static auto name() -> const char*;
    static auto namecall_if(state_t L, Ty& self, int atom) -> std::optional<int>;
};
template <class Ty>
concept has_get_method = requires (Ty v) {v.get();};
template <class Ty, has_get_method Owning = std::shared_ptr<Ty>>
struct interface {
    using type = Ty;
    using pointer = Ty*;
    using reference = Ty&;
    std::variant<pointer, Owning> ptr;
    interface(pointer ptr): ptr(ptr) {}
    interface(Owning ptr): ptr(std::move(ptr)) {}
    constexpr auto get() -> pointer {
        if (auto p = std::get_if<Ty*>(&ptr)) return *p;
        else return std::get<Owning>(ptr).get();
    }
    constexpr auto operator->() -> pointer {return get();}
    constexpr auto operator*() -> reference {return *get();}
};
using path = type_impl<std::filesystem::path>;
using path_t = path::type;
using filewriter = type_impl<std::ofstream>;
using filewriter_t = filewriter::type;
using writer = type_impl<interface<std::ostream>>;
using writer_t = writer::type;
using string_opt = std::optional<std::string>;
template<class Ty>
struct lib_impl {
    static auto name() -> std::string;
    static void push(state_t L);
    static void open(state_t L) {
        push(L);
        lua_setglobal(L, name().c_str());
    }
    static void as_field(state_t L, int idx = -2, std::string const& field = name()) {
        push(L);
        lua_setfield(L, idx, field.c_str());
    }
};
namespace lib {
using filesystem = lib_impl<struct filesystem_tag>;
using io = lib_impl<struct io_tag>;
using process = lib_impl<struct process_tag>;
using garbage = lib_impl<struct garbage_tag>;
}
auto push_directory_iterator(state_t L, path_t const& directory, bool recursive) -> int;
auto to_path(state_t L, int idx) -> path_t;
auto init_state(const char* libname = "lib") -> state_owner_t;
auto load_script(state_t L, const std::filesystem::path& path) -> std::expected<state_t, std::string>;
void open_require(state_t L);
struct require_context {
    std::filesystem::path absolute_path;
    std::filesystem::path path;
    std::string suffix;
    bool codegen_enabled;
};
auto current_require_context(state_t L) -> require_context;
inline void push_api(state_t L, std::span<const luaL_Reg> api) {
    lua_newtable(L);
    for (auto const& entry : api) {
        lua_pushcfunction(L, entry.func, entry.name);
        lua_setfield(L, -2, entry.name);
    }
}

enum class read_file_error {
    not_a_file,
    failed_to_open,
    failed_to_read,
};;
inline auto read_file(path_t const& path) -> std::expected<std::string, read_file_error> {
    using err = read_file_error;
    if (not std::filesystem::is_regular_file(path)) {
        return std::unexpected(err::not_a_file);
    }
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    if (!file) return std::unexpected(err::failed_to_open);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);
    auto buf = std::string(size, '\0');
    if (not file.read(&buf.data()[0], size)) {
        return std::unexpected(err::failed_to_read);
    }
    return buf;
}
template <class Ty>
concept compile_options_ish = requires (Ty v) {
    v.optimizationLevel;
    v.debugLevel;
    v.typeInfoLevel;
    v.coverageLevel;
    v.userdataTypes;
};

template <class Ty = lua_CompileOptions>
requires compile_options_ish<Ty>
static auto compile_options() -> Ty {
    static const char* userdata_types[] = {
        path::name(),
        filewriter::name(),
        nullptr
    };
    auto opts = Ty{};
    opts.optimizationLevel = 2;
    opts.debugLevel = 3;
    opts.typeInfoLevel = 2;
    opts.coverageLevel = 2;
    opts.userdataTypes = userdata_types;
    return opts;
}

struct args_wrapper {
    int argc;
    char** argv;
    auto span() const -> std::span<char*> {
        return {argv, argv + argc};
    }
    auto view() const -> decltype(auto) {
        return std::views::transform(span(), [](auto v) -> std::string_view {return v;});
    }
    auto operator[](size_t idx) const -> std::optional<std::string_view> {
        if (idx >= argc) return std::nullopt;
        return argv[idx];
    }
    args_wrapper(int argc, char** argv): argc(argc), argv(argv) {}
    explicit args_wrapper() = default;
};
namespace internal {
    auto get_args() -> args_wrapper const&;
}
#endif
