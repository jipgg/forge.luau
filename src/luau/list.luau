type data<T> = {
    [number]: T,
    _array: {T},
}
type impl<T> = {
    filter: (self: list<T>, pred: (e: T) -> boolean) -> list<T>,
    append: (self: list<T>, v: T) -> list<T>,
    pop_back: (self: list<T>) -> list<T>,
}
type meta<T> = {
    __index: index<T>,
    __newindex: (self: list<T>, idx: number, value: number) -> (),
    __add: (self: list<T>, T | {T}) -> list<T>,
    __len: (self: list<T>) -> number,
}
type indexer<T> = {
    __index: (index<T>, idx: number) -> T,
}
export type index<T> = typeof(setmetatable({} :: impl<T>, {} :: indexer<T>))
export type list<T> = typeof(setmetatable({} :: data<T>, {} :: meta<T>))
local index = setmetatable({}, {
    __index = function(self: any, v: number)
        return self._array[v]
    end
}) :: index<any>
local meta = {__index = index} :: meta<any>
function index:filter(pred)
    for i, v in self._array do
    end
end

return function<T>(...: T): list<T>
    local SIZE = select('#', ...)
    local array = table.create(SIZE)
    for i=1, SIZE do
        array[i] = select(i, ...)
    end
    local data: data<T> = {_array = array}
    return setmetatable(data, meta)
end
