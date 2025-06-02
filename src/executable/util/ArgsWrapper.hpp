#pragma once
#include <span>
#include <optional>
#include <string_view>
#include <ranges>

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
