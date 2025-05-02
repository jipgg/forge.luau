#ifndef UTILITY_COMPILE_TIME_HPP
#define UTILITY_COMPILE_TIME_HPP
#include <source_location>
#include <utility>
#include <string_view>
#include <ranges>
#include <string>
#include <algorithm>
#include <type_traits>

namespace comptime {
template<class type, type v>
struct comptime_value {
    static const constexpr auto value = v;
    constexpr operator type() const {return v;}
};
template <class type, std::size_t... indices>
constexpr void unroll_for(type fn, std::index_sequence<indices...>) {
  (fn(comptime_value<std::size_t, indices>{}), ...);
}
template <std::size_t Count, typename Func>
constexpr void unroll_for(Func fn) {
  unroll_for(fn, std::make_index_sequence<Count>());
}
template <class type>
concept sentinel_enum = requires {
    std::is_enum_v<type>;
    type::comptime_sentinel_keyword;
};
template <class from, class to>
concept castable_to = requires {
    static_cast<to>(from{});
};
template <sentinel_enum type>
consteval std::size_t enum_size() {
    return static_cast<std::size_t>(type::comptime_sentinel_keyword);
}
template <sentinel_enum type, type last_item>
consteval std::size_t enum_size() {
    return static_cast<std::size_t>(last_item) + 1;
}
struct enum_info_t {
    std::string_view type;
    std::string_view name;
    std::string_view raw;
    int value;
};
template <sentinel_enum type>
struct enum_item_t {
    std::string_view name;
    int value;
    constexpr type to_enum() const {return type(value);}
    constexpr operator type() const {return type(value);}
    template<int size = enum_size<type>>
    enum_item_t(auto value);
};
namespace detail {
#if defined (_MSC_VER)//msvc compiler
template <sentinel_enum type, type value>
consteval enum_info_t enum_info_impl() {
    const std::string_view raw{std::source_location::current().function_name()};
    const std::string enum_t_keyw{"<enum "};
    auto found = raw.find(enum_t_keyw);
    std::string_view enum_t = raw.substr(raw.find(enum_t_keyw) + enum_t_keyw.size());
    bool is_enum_class = enum_t.find("::") != std::string::npos;
    enum_t = enum_t.substr(0, enum_t.find_first_of(','));
    const std::string preceding = enum_t_keyw + std::string(enum_t) + ",";
    std::string_view enum_v = raw.substr(raw.find(preceding) + preceding.size());
    if (enum_v.find("::")) {
        enum_v = enum_v.substr(enum_v.find_last_of("::") + 1);
    }
    enum_v = enum_v.substr(0, enum_v.find_first_of(">"));
    return {
        .type = enum_t,
        .name = enum_v,
        .raw = raw,
        .value = static_cast<int>(value),
    };
}
#elif defined(__clang__) || defined(__GNUC__)
template <sentinel_enum type, type value>
consteval enum_info_t enum_info_impl() {
    using sv = std::string_view;
    const sv raw{std::source_location::current().function_name()};
    const sv enum_find{"type = "}; 
    const sv value_find{"value = "};
    sv enum_t = raw.substr(raw.find(enum_find) + enum_find.size());
    enum_t = enum_t.substr(0, enum_t.find(";"));
    sv enum_v = raw.substr(raw.find(value_find) + value_find.size());
    if (enum_v.find("::")) {
        enum_v = enum_v.substr(enum_v.find_last_of("::") + 1);
    }
    enum_v = enum_v.substr(0, enum_v.find("]"));
    return {
        .type = enum_t,
        .name = enum_v,
        .raw = raw,
        .value = static_cast<int>(value),
    };
}
#endif
}
template <auto value, sentinel_enum type = decltype(value)>
consteval enum_info_t enum_info() {
    return detail::enum_info_impl<type, static_cast<type>(value)>();
}
template <auto value, sentinel_enum type = decltype(value)>
constexpr enum_item_t<type> enum_item() {
    constexpr auto i = enum_info<value, type>();
    return enum_item_t<type>{
        .name = i.name,
        .value = static_cast<int>(value),
    };
}
template <sentinel_enum type, int size = enum_size<type>()> 
consteval std::array<enum_info_t, size> to_array() {
    std::array<enum_info_t, size> arr{};
    unroll_for<size>([&arr](auto i) {
        arr[i] = enum_info<static_cast<type>(i.value)>();
    });
    return arr;
}
template <sentinel_enum type, int size = enum_size<type>()>
constexpr enum_item_t<type> enum_item(std::string_view name) {
    constexpr auto array = to_array<type, size>();
    auto found_it = std::ranges::find_if(array, [&name](const enum_info_t& e) {return e.name == name;});
    assert(found_it != std::ranges::end(array));
    return {
        .name = found_it->name,
        .value = found_it->value
    };
}
template <sentinel_enum type, int size = enum_size<type>()>
constexpr enum_item_t<type> enum_item(auto value) {
    constexpr auto array = to_array<type, size>();
    const auto& info = array.at(static_cast<size_t>(value)); 
    return {
        .name = info.name,
        .value = info.value
    };
}
}
#endif
