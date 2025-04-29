#ifndef UTILITY_SCOPE_DEFERRED_HPP
#define UTILITY_SCOPE_DEFERRED_HPP
#include <functional>

struct scope_defer {
    using fn = std::function<void()>;
    auto operator=(const scope_defer&) -> scope_defer& = delete;
    scope_defer(const scope_defer&) = delete;
    ~scope_defer() {fn_();}
    explicit scope_defer(fn fn): fn_() {}
private:
    fn fn_;
};

#endif
