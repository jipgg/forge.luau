#ifndef COMMON_HPP
#define COMMON_HPP

#include <memory>
#include <expected>
#include <filesystem>
#include <lualib.h>
#include "utility/compile_time.hpp"
#include "utility/luau.hpp"
#include "utility/library_config.hpp"
#include <fstream>
#include <cassert>
using luau::state_t;
using luau::state_owner_t;

namespace lib {
    template<class Ty>
    struct impl {
        static void open(state_t L, library_config config);
    };
    using filesystem = impl<struct filesystem_tag>;
    using fileio = impl<struct fileio_tag>;
    using consoleio = impl<struct consoleio_tag>;
    using process = impl<struct process_tag>;
    using json = impl<struct json_tag>;
}
namespace type {
    template<class Ty>
    struct impl {
        using type = Ty;
        using util = luau::generic_userdatatagged_builder<Ty>;
        static void init(state_t L);
        static auto name() -> const char*;
        static auto namecall_if(state_t L, Ty& self, int atom) -> std::optional<int>;
    };
    using path = impl<std::filesystem::path>;
    using path_t = path::type;
    using filewriter = impl<std::ofstream>;
    using filewriter_t = filewriter::type;
}
namespace state {
    auto init(const char* name = "lib") -> state_owner_t;
    auto load_script(state_t L, const std::filesystem::path& path) -> std::expected<state_t, std::string>;
}
auto to_path(state_t L, int idx) -> type::path_t;

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
namespace detail {
    template <class Iterator>
    inline auto directory_iterator_closure(state_t L) -> int {
        auto& it = luau::to_userdata<Iterator>(L, lua_upvalueindex(1));
        const Iterator end{};
        if (it != end) {
            const std::filesystem::directory_entry& entry = *it;
            auto path = entry.path();
            type::path::util::push(L, path);
            ++it;
            return 1;
        }
        return 0;
    }
}
inline auto push_directory_iterator(state_t L, std::filesystem::path const& directory, bool recursive) -> int {
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
#endif
