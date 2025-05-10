#pragma once
#include <expected>
#include <filesystem>
#include <lualib.h>
#include <Luau/Require.h>
#include "util/lua.hpp"
#include <fstream>
#include <expected>
#include <cassert>
#include <variant>
#include "util/type_utility.hpp"
#include <httplib.h>
using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using i8 = std::uint8_t;
using i16 = std::uint16_t;
using i32 = std::uint32_t;
using f32 = float;
using f64 = double;
auto result_create(lua_State* L) -> int;
auto push_directory_iterator(lua_State* L, std::filesystem::path const& directory, bool recursive) -> int;
auto to_path(lua_State* L, int idx) -> std::filesystem::path;
auto push_path(lua_State* L, std::filesystem::path const& path) -> int;
auto init_state(const char* libname = "lib") -> lua::StateOwner;
auto load_script(lua_State* L, const std::filesystem::path& path) -> std::expected<lua_State*, std::string>;
void open_require(lua_State* L);
void open_fslib(lua_State* L);
void open_iolib(lua_State* L);
void open_oslib(lua_State* L);
void open_httplib(lua_State* L);
void open_jsonlib(lua_State* L);

inline void push_library(lua_State* L, std::span<const luaL_Reg> api) {
    lua_newtable(L);
    for (auto const& entry : api) {
        lua_pushcfunction(L, entry.func, entry.name);
        lua_setfield(L, -2, entry.name);
    }
}
enum class ReadFileError {
    not_a_file,
    failed_to_open,
    failed_to_read,
};;
inline auto read_file(std::filesystem::path const& path) -> std::expected<std::string, ReadFileError> {
    using err = ReadFileError;
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
template <class T>
concept CompileOptionsIsh = requires (T v) {
    v.optimizationLevel;
    v.debugLevel;
    v.typeInfoLevel;
    v.coverageLevel;
    v.userdataTypes;
};

template <class T>
struct Interface {
    using Type = T;
    enum class Holds {
        pointer,
        shared_ptr,
    };
    Interface(T& ref): holding_(&ref), holds_(Holds::pointer) {}
    Interface(T* ptr): holding_(ptr), holds_(Holds::pointer) {}
    Interface(std::shared_ptr<T> ptr): holding_(std::move(ptr)), holds_(Holds::shared_ptr) {}
    constexpr auto get() -> T* {
        switch (holds_) {
            case Holds::pointer:
                return std::get<T*>(holding_);
            default:
                return std::get<std::shared_ptr<T>>(holding_).get();
        }
    }
    constexpr auto operator->() -> T* {return get();}
    constexpr auto operator*() -> T& {return *get();}
private:
    std::variant<T*, std::shared_ptr<T>> holding_;
    Holds holds_;
};
using Path = std::filesystem::path;
using Writer = Interface<std::ostream>;
using Reader = Interface<std::istream>;
using FileWriter = std::ofstream;
using HttpClient = httplib::Client;

template <CompileOptionsIsh T = lua_CompileOptions>
static auto compile_options() -> T {
    static const char* userdata_types[] = {
        Type<Writer>::config.tname(),
        Type<FileWriter>::config.tname(),
        Type<Path>::config.tname(),
        nullptr
    };
    auto opts = T{};
    opts.optimizationLevel = 2;
    opts.debugLevel = 3;
    opts.typeInfoLevel = 2;
    opts.coverageLevel = 2;
    //opts.userdataTypes = userdata_types;
    return opts;
}
struct ArgsWrapper {
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
    ArgsWrapper(int argc, char** argv): argc(argc), argv(argv) {}
    explicit ArgsWrapper() = default;
};
namespace internal {
    auto get_args() -> ArgsWrapper const&;
}
