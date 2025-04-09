#ifndef USERDATA_HPP
#define USERDATA_HPP
#include "lualib.h"
#include <concepts>
#include <functional>
#define register_type_name(t, n)\
template<> const char* ::userdata::Taginfo<t>::name{n};

namespace userdata {
using state_t = lua_State*;
namespace detail {
    inline int new_tag() {
        static int tag{};
        return ++tag;
    }
}
template <class Ty>
struct Taginfo {
    inline static int tag = detail::new_tag();
    static const char* name;
};
template <class Ty>
constexpr auto tag_for() -> int {
    return Taginfo<Ty>::tag;
}
template <class Ty>
constexpr auto name_for() -> const char* {
    return Taginfo<Ty>::name;
}
template<class Ty>
auto get(state_t L, int idx) -> Ty& {
    auto p = static_cast<Ty*>(lua_touserdatatagged(L, idx, tag_for<Ty>()));
    return *p;
}
template<class Ty>
auto self(state_t L) -> Ty& {
    return get<Ty>(L, 1);
}
template<class Ty>
auto is_type(state_t L, int idx) -> bool {
    return lua_userdatatag(L, idx) == tag_for<Ty>();
}
template<class Ty>
auto check(state_t L, int idx) -> Ty& {
    if (not is_type<Ty>(L, idx)) luaL_typeerrorL(L, idx, name_for<Ty>());
    return get<Ty>(L, idx);
}
template<class Ty>
auto get_if(state_t L, int idx) -> Ty* {
    if (not is_type<Ty>(L, idx)) return nullptr;
    return static_cast<Ty*>(lua_touserdatatagged(L, idx, tag_for<Ty>()));
}
template<class Ty>
auto or_else(state_t L, int idx, std::function<Ty()> fn) -> Ty {
    auto v = get_if<Ty>(L, idx);
    return v ? *v : fn();
} 
template<class Ty>
void basic_dtor(state_t, void* p) {
    static_cast<Ty*>(p)->~Ty();
}
template<class Ty>
void set_dtor(state_t L, lua_Destructor dtor = basic_dtor<Ty>) {
    lua_setuserdatadtor(L, tag_for<Ty>(), dtor);
}

template<class Ty, class ...Args>
requires std::constructible_from<Ty, Args&&...>
auto make(state_t L, Args&&...args) -> Ty& {
    auto p = static_cast<Ty*>(lua_newuserdatatagged(L, sizeof(Ty), tag_for<Ty>()));
    std::construct_at(p, std::forward<Args>(args)...);
    luaL_getmetatable(L, name_for<Ty>());
    lua_setmetatable(L, -2);
    return *p;
}
template <class Ty>
auto push(state_t L, const Ty& v) -> int {
    make<Ty>(L, v);
    return 1;
}
}
#endif
