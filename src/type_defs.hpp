#pragma once
#include <variant>
#include <lua.h>
#include <memory>
#include <filesystem>
#include <span>
#include <ranges>

template <class type>
struct type_wrapper {
    using holding_type = type;
    enum class holds {
        pointer,
        shared_ptr,
    };
    type_wrapper(type& ref): holding_(&ref), holds_(holds::pointer) {}
    type_wrapper(type* ptr): holding_(ptr), holds_(holds::pointer) {}
    type_wrapper(std::shared_ptr<type> ptr): holding_(std::move(ptr)), holds_(holds::shared_ptr) {}
    constexpr auto get() -> type* {
        switch (holds_) {
            case holds::pointer:
                return std::get<type*>(holding_);
            default:
                return std::get<std::shared_ptr<type>>(holding_).get();
        }
    }
    constexpr auto operator->() -> type* {return get();}
    constexpr auto operator*() -> type& {return *get();}
private:
    std::variant<type*, std::shared_ptr<type>> holding_;
    holds holds_;
};
using path = std::filesystem::path;
using writer = type_wrapper<std::ostream>;
using file_writer = std::ofstream;
struct result {
    int ref;
    bool has_value;
    auto push(lua_State* L) -> int {
        lua_getref(L, ref);
        return 1;
    }
    static auto destructor(lua_State* L, void* userdata) {
        auto* self = static_cast<result*>(userdata);
        lua_unref(L, self->ref);
    }
};
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
