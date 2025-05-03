local result = require('./result')
type result<V, E> = result.type<V, E>
type data<T> = {
    _array: {T},
}

type impl<T> = {
    push: (self: stack<T>, v: T) -> (),
    push_range: (self: stack<T>, ...T) -> (),
    top: (self: stack<T>) -> T,
    empty: (self: stack<T>) -> boolean,
    any: (self: stack<T>) -> boolean,
    pop: (self: stack<T>) -> T,
    pop_range: (self: stack<T>, amount: number) -> (...T),
}
type meta<T> = {
    __index: impl<T>,
}

export type stack<T> = typeof(setmetatable({} :: data<T>, {} :: meta<T>))

return function<T>(...: T): stack<T>
    return (nil :: any) :: stack<T>
end
