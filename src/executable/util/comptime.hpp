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
template<typename T, T Val>
struct Value {
    static const constexpr auto value = Val;
    constexpr operator T() const {return Val;}
};
template <typename T, std::size_t... Indices>
constexpr void unroll_for(T fn, std::index_sequence<Indices...>) {
  (fn(Value<std::size_t, Indices>{}), ...);
}
template <std::size_t Count, typename Func>
constexpr void unroll_for(Func fn) {
  unroll_for(fn, std::make_index_sequence<Count>());
}
template <typename type>
concept SentinelEnum = requires {
    std::is_enum_v<type>;
    type::comptime_sentinel_keyword;
};
template <typename from, typename to>
concept CastableTo = requires {
    static_cast<to>(from{});
};
template <SentinelEnum type>
consteval std::size_t enum_size() {
    return static_cast<std::size_t>(type::comptime_sentinel_keyword);
}
template <SentinelEnum T, T Last>
consteval std::size_t enum_size() {
    return static_cast<std::size_t>(Last) + 1;
}
struct EnumInfo {
    std::string_view type;
    std::string_view name;
    std::string_view raw;
    int value;
};
template <SentinelEnum type>
struct EnumItem {
    std::string_view name;
    int value;
    constexpr type to_enum() const {return type(value);}
    constexpr operator type() const {return type(value);}
    template<int size = enum_size<type>>
    EnumItem(auto value);
};
namespace detail {
#if defined (_MSC_VER)//msvc compiler
template <SentinelEnum T, T Value>
consteval EnumInfo enum_info_impl() {
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
        .value = static_cast<int>(Value),
    };
}
#elif defined(__clang__) || defined(__GNUC__)
template <SentinelEnum T, T Value>
consteval enum_info_t enum_info_impl() {
    using sv = std::string_view;
    const sv raw{std::source_location::current().function_name()};
    const sv enum_find{"T = "}; 
    const sv value_find{"Value = "};
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
        .value = static_cast<int>(Value),
    };
}
#endif
}
template <auto Value, SentinelEnum T = decltype(Value)>
consteval EnumInfo enum_info() {
    return detail::enum_info_impl<T, static_cast<T>(Value)>();
}
template <auto Value, SentinelEnum T = decltype(Value)>
constexpr EnumItem<T> enum_item() {
    constexpr auto i = enum_info<Value, T>();
    return EnumItem<T>{
        .name = i.name,
        .value = static_cast<int>(Value),
    };
}
template <SentinelEnum T, int Size = enum_size<T>()> 
consteval std::array<EnumInfo, Size> to_array() {
    std::array<EnumInfo, Size> arr{};
    unroll_for<Size>([&arr](auto i) {
        arr[i] = enum_info<static_cast<T>(i.value)>();
    });
    return arr;
}
template <SentinelEnum T, int Size = enum_size<T>()>
constexpr EnumItem<T> enum_item(std::string_view name) {
    constexpr auto array = to_array<T, Size>();
    auto found_it = std::ranges::find_if(array, [&name](const EnumInfo& e) {return e.name == name;});
    assert(found_it != std::ranges::end(array));
    return {
        .name = found_it->name,
        .value = found_it->value
    };
}
template <SentinelEnum T, int Size = enum_size<T>()>
constexpr EnumItem<T> enum_item(auto value) {
    constexpr auto array = to_array<T, Size>();
    const auto& info = array.at(static_cast<std::size_t>(value)); 
    return {
        .name = info.name,
        .value = info.value
    };
}
}
#endif
