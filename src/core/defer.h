#ifndef CORE_DEFER_H_
#define CORE_DEFER_H_

#define DEFER_CAT_(A, B) A ## B ## _
#define DEFER_CAT(A, B) DEFER_CAT_(A, B)

template <typename Func>
struct Defer {
    Func fn;
    Defer() = default;
    Defer(Func fn_) : fn(fn_) {}
    ~Defer() { fn(); }
};

// Executes expression at the end of the enclosing scope.
#define defer(expr) auto DEFER_CAT(defer_exp_, __LINE__) = Defer([&] { expr; })

#endif // CORE_DEFER_H_
