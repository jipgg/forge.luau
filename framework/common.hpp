#ifndef COMMON_H
#define COMMON_H
#include <concepts>
#include <memory>
#include "lualib.h"
using state_t = lua_State*;
using state_owner_t = std::unique_ptr<lua_State, decltype(&lua_close)>;
namespace common {
constexpr auto push_api(state_t L, const luaL_Reg* api) {
    lua_newtable(L);
    luaL_register(L, nullptr, api);
}
constexpr auto set_api(state_t L, const luaL_Reg* api, const std::string& name, int idx = -1) {
    lua_newtable(L);
    luaL_register(L, nullptr, api);
}
template <class From, class To>
concept Castable_To = requires {
    static_cast<From>(To{});
};
template <class Ty>
constexpr auto to_userdata(state_t L, int idx) -> Ty& {
    return *static_cast<Ty*>(lua_touserdata(L, idx));
}
template <class Atom = int>
requires Castable_To<Atom, int>
constexpr auto namecall_atom(state_t L) -> std::pair<Atom, const char*> {
    int atom{};
    const char* name = lua_namecallatom(L, &atom);
    return {static_cast<Atom>(atom), name};
}
template <auto Tag>
requires Castable_To<decltype(Tag), int>
constexpr auto has_userdata_tag(state_t L, int idx) -> int {
    return lua_userdatatag(L, idx) == static_cast<int>(Tag);
}
template <class Ty, auto Tag>
requires Castable_To<decltype(Tag), int>
constexpr auto to_userdata_tagged(state_t L, int idx) -> Ty& {
    return *static_cast<Ty*>(lua_touserdatatagged(L, idx, static_cast<int>(Tag)));
}
namespace detail {
template <class Ty>
void default_dtor(void* ud) {
    static_cast<Ty*>(ud)->~Ty();
}
}
template <class Ty,  class ...Params>
requires std::constructible_from<Ty, Params&&...>
auto make_userdata(lua_State* L, Params&&...args) -> Ty& {
    Ty* ud = static_cast<Ty*>(lua_newuserdatadtor(L, sizeof(Ty), detail::default_dtor<Ty>));
    std::construct_at(ud, std::forward<Params>(args)...);
    return *ud;
}
template <class Ty, auto Tag,  class ...Params>
requires std::constructible_from<Ty, Params&&...>
and Castable_To<decltype(Tag), int>
auto make_userdata_tagged(lua_State* L, Params&&...args) -> Ty& {
    Ty* ud = static_cast<Ty*>(lua_newuserdatatagged(L, sizeof(Ty), static_cast<int>(Tag)));
    std::construct_at(ud, std::forward<Params>(args)...);
    return *ud;
}
class Ref {
    int ref_;
    state_t state_;
public:
    Ref(): ref_(-1), state_(nullptr) {}
    Ref(state_t L, int idx):
        ref_(lua_ref(L, idx)),
        state_(lua_mainthread(L)) {
    }
    Ref(const Ref& other) {
        state_ = other.state_;
        if (state_) {
            push(state_);
            ref_ = lua_ref(state_, -1);
            lua_pop(state_, 1);
        }
    }
    Ref(Ref&& other) noexcept {
        state_ = other.state_;
        ref_ = other.ref_;
        other.state_ = nullptr;
    }
    Ref& operator=(const Ref& other) {
        state_ = other.state_;
        if (state_) {
            push(state_);
            ref_ = lua_ref(state_, -1);
            lua_pop(state_, 1);
        }
        return *this;
    }
    Ref& operator=(Ref&& other) noexcept {
        state_ = other.state_;
        ref_ = other.ref_;
        other.state_ = nullptr;
        return *this;
    }
    ~Ref() {
        if (state_) {
            lua_unref(state_, ref_);
        }
    }
    operator bool() const {
        return state_;
    }
    void push(lua_State* L) {
        if (not state_) {
            lua_pushnil(L);
            return;
        }
        lua_getref(L, ref_);
    }
    void release() {
        state_ = nullptr;
    }
};
}
#endif
