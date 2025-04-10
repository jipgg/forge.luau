#ifndef UTILITY_SCOPE_DEFERRED_HPP
#define UTILITY_SCOPE_DEFERRED_HPP
#include <functional>

struct scope_defer {
    using fn_t = std::function<void()>;
    auto operator=(const scope_defer&) -> scope_defer& = delete;
    scope_defer(const scope_defer&) = delete;
    ~scope_defer() {_fn();}
    explicit scope_defer(fn_t fn): _fn() {}
private:
    fn_t _fn;
};

#endif
