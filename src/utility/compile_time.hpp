#ifndef UTILITY_COMPILE_TIME_HPP
#define UTILITY_COMPILE_TIME_HPP
#include <source_location>
#include <utility>
#include <string_view>
#include <ranges>
#include <algorithm>

namespace compile_time {
template<class Val, Val v>
struct Value {
    static const constexpr auto value = v;
    constexpr operator Val() const {return v;}
};
template <class Func, std::size_t... Indices>
constexpr void unroll_for(Func fn, std::index_sequence<Indices...>) {
  (fn(Value<std::size_t, Indices>{}), ...);
}
template <std::size_t Count, typename Func>
constexpr void unroll_for(Func fn) {
  unroll_for(fn, std::make_index_sequence<Count>());
}
#define COMPILE_TIME_ENUM_SENTINEL _end
template <class Ty>
concept Sentinel_Enum = requires {
    std::is_enum_v<Ty>;
    Ty::COMPILE_TIME_ENUM_SENTINEL;
};
template <class From, class To>
concept Static_Castable_To = requires {
    static_cast<To>(From{});
};
template <Sentinel_Enum Type>
consteval std::size_t count() {
    return static_cast<std::size_t>(Type::COMPILE_TIME_ENUM_SENTINEL);
}
template <Sentinel_Enum Type, Type Last_Item>
consteval std::size_t count() {
    return static_cast<std::size_t>(Last_Item) + 1;
}
struct Enum_Info {
    std::string_view type;
    std::string_view name;
    std::string_view raw;
    int value;
};
template <Sentinel_Enum Type>
struct Enum_Item {
    std::string_view name;
    int value;
    constexpr Type to_enum() const {return Type(value);}
    constexpr operator Type() const {return Type(value);}
};
namespace detail {
#if defined (_MSC_VER)//msvc compiler
template <Sentinel_Enum Ty, Ty Val>
consteval Enum_Info enum_info() {
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
        .value = static_cast<int>(Val),
    };
}
#elif defined(__clang__) || defined(__GNUC__)
template <Sentinel_Enum Ty, Ty Val>
consteval Enum_Info enum_info() {
    using sv = std::string_view;
    const sv raw{std::source_location::current().function_name()};
    const sv enum_find{"Ty = "}; 
    const sv value_find{"Val = "};
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
        .value = static_cast<int>(Val),
    };
}
#endif
}
template <auto Val, Sentinel_Enum Ty = decltype(Val)>
consteval Enum_Info enum_info() {
    return detail::enum_info<Ty, static_cast<Ty>(Val)>();
}
template <auto Val, Sentinel_Enum Ty = decltype(Val)>
constexpr Enum_Item<Ty> enum_item() {
    constexpr auto info = enum_info<Val, Ty>();
    return Enum_Item<Ty>{
        .name = info.name,
        .value = static_cast<int>(Val),
    };
}
template <Sentinel_Enum Ty, int Count = count<Ty>()> 
consteval std::array<Enum_Info, Count> to_array() {
    std::array<Enum_Info, Count> arr{};
    unroll_for<Count>([&arr](auto i) {
        arr[i] = enum_info<static_cast<Ty>(i.value)>();
    });
    return arr;
}
template <Sentinel_Enum Type, int Size = count<Type>()>
constexpr Enum_Item<Type> enum_item(std::string_view name) {
    constexpr auto array = to_array<Type, Size>();
    auto found_it = std::ranges::find_if(array, [&name](const Enum_Info& e) {return e.name == name;});
    assert(found_it != std::ranges::end(array));
    return {
        .name = found_it->name,
        .value = found_it->value
    };
}
template <Sentinel_Enum Type, int Size = count<Type>()>
constexpr Enum_Item<Type> enum_item(auto value) {
    constexpr auto array = to_array<Type, Size>();
    const auto& info = array.at(static_cast<size_t>(value)); 
    return {
        .name = info.name,
        .value = info.value
    };
}
}
#endif
