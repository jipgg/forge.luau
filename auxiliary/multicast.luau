local impl = {} :: _Multicast_impl<...any>
impl.__index = impl
impl.error_handler = function(ref, ...)
    print(`Error in {ref}: {debug.traceback()}`)
end
function impl:clear()
    table.clear(self.functions)
end
function impl:empty()
    return #self.functions == 0
end
function impl:size()
    return #self.functions
end

function impl:__add(fn)
    table.insert(self.functions, fn)
    return self
end
function impl:__sub(fn)
    local found = table.find(self.functions, fn)
    if found then
        table.remove(self.functions, found)
    end
    return self
end
function impl:__call(...)
    for _, fn in self.functions do
        if not pcall(fn, ...) then
            (self.error_handler :: any)(fn, ...)
        end
    end
end

export type _Multicast<As...> = {
    functions: {(As...)->(...any)},
    error_handler: ((ref: (As...)->(...any), As...) -> ())?,
}
export type _Multicast_impl<As...> = {
    __index: _Multicast_impl<As...>,
    __call: (multicast<As...>, As...) -> (),
    __add: (multicast<As...>, fn: (As...)->(...any)) -> multicast<As...>,
    __sub: (multicast<As...>, fn: (As...)->(...any)) -> multicast<As...>,
    clear: (multicast<As...>) -> (),
    empty: (multicast<As...>) -> boolean,
    size: (multicast<As...>) -> number,
    error_handler: (ref: (As...)->(...any), As...) -> (),
}
export type multicast<As... = ...any> = typeof(setmetatable(
    {} :: _Multicast<As...>,
    {} :: _Multicast_impl<As...>
))

local function make<As...>(fns: _Multicast<As...>?): multicast<As...>
    local self: _Multicast<As...> = {
        functions = fns or {}
    }
    return setmetatable(self, impl) :: any
end
return make
