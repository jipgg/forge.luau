#ifndef UTILITY_SCOPE_DEFERRED_HPP
#define UTILITY_SCOPE_DEFERRED_HPP
#include <functional>

struct ScopeDefer {
    using fn = std::function<void()>;
    auto operator=(const ScopeDefer&) -> ScopeDefer& = delete;
    ScopeDefer(const ScopeDefer&) = delete;
    ~ScopeDefer() {fn_();}
    explicit ScopeDefer(fn fn): fn_() {}
private:
    fn fn_;
};

#endif
