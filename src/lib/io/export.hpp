#pragma once
#include <fstream>
#include <iostream>
#include <variant>
struct lua_State;

namespace lib::io {
template <typename T>
struct interface {
    using type = T;
    enum class holds {
        pointer,
        shared_ptr,
    };
    interface(T& ref): holding_(&ref), holds_(holds::pointer) {}
    interface(T* ptr): holding_(ptr), holds_(holds::pointer) {}
    interface(std::shared_ptr<T> ptr): holding_(std::move(ptr)), holds_(holds::shared_ptr) {}
    constexpr auto get() -> T* {
        switch (holds_) {
            case holds::pointer:
                return std::get<T*>(holding_);
            default:
                return std::get<std::shared_ptr<T>>(holding_).get();
        }
    }
    constexpr auto operator->() -> T* {return get();}
    constexpr auto operator*() -> T& {return *get();}
private:
    std::variant<T*, std::shared_ptr<T>> holding_;
    holds holds_;
};
using writer = interface<std::ostream>;
using reader = interface<std::istream>;
using filewriter = std::ofstream;
using filereader = std::ifstream;
void library(lua_State* L, int idx);
}
