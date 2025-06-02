#pragma once
#include <memory>
#include <variant>

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
