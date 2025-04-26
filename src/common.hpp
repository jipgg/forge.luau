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
using path = type_impl<std::filesystem::path>;
using path_t = path::type;
using filewriter = type_impl<std::ofstream>;
using filewriter_t = filewriter::type;
template<class Ty>
struct lib_impl {
    static void open(state_t L, library_config config);
};
using filesystem = lib_impl<struct filesystem_tag>;
using fileio = lib_impl<struct fileio_tag>;
using consoleio = lib_impl<struct consoleio_tag>;
using process = lib_impl<struct process_tag>;
using json = lib_impl<struct json_tag>;
auto push_directory_iterator(state_t L, path_t const& directory, bool recursive) -> int;
auto to_path(state_t L, int idx) -> path_t;
auto init_state(const char* libname = "lib") -> state_owner_t;
auto load_script(state_t L, const std::filesystem::path& path) -> std::expected<state_t, std::string>;

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
namespace internal {
    auto get_args() -> args_wrapper const&;
}
#endif
