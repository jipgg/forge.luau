--!strict
export type data<V, E> = {
    _ok: boolean,
    _v: V | E,
}
export type index<V, E> = {
    and_then: (result<V, E>, (V) -> result<V, E>) -> result<V, E>,
    unpack: (result<V, E>) -> (V?, E?),
    value: (result<V, E>) -> V | never,
    error: (result<V, E>) -> E,
    has_value: (result<V, E>) -> boolean,
    has_error:(result<V, E>) -> boolean,
    value_or: (result<V, E>, V) -> V, 
    or_else: (result<V, E>, (E) -> result<V, E>) -> result<V, E>,
    error_or: (result<V, E>, E) -> E, 
    match: <T>(result<V, E>, (V) -> T, (E) -> T) -> T,
    switch: (result<V, E>, (V) -> (), (E) -> ()) -> (),
}
export type meta<V, E> = {
    __index: index<V, E>
}
type result<V, E> = typeof(setmetatable({} :: data<V, E>, {} :: meta<V, E>))
export type type<V, E> = result<V, E>
local index = {} :: index<any, any>
local meta = {__index = index} :: meta<any, any>
function index:and_then(fn)
    return self._ok and fn(self._v) or self
end
function index:or_else(fn)
    return not self._ok and fn(self._v) or self
end
function index:value()
    return self._ok and self._v or error("bad access")
end
function index:error()
    return not self._ok and self._v or error("bad access")
end
function index:has_value()
    return self._ok
end
function index:has_error()
    return not self._ok
end
function index:value_or(v)
    return self._ok and self._v or v
end
function index:error_or(v)
    return not self._ok and self._v or v
end
function index:match<T>(v, e)
    return self._ok and v(self._v) or e(self._v)
end
function index:switch(v, e)
    return self._ok and v(self._v) or e(self._v)
end

function index:unpack()
    if self._ok then
        return self._v, nil;
    else
        return nil, self._v;
    end
end

local type = {}
function type:__call(v, e)
end

local function create<V, E>(v, e)
    local data: data<V, E> = {
        _ok = v ~= nil,
        _v = v and v or e,
    }
    return setmetatable(data, meta)
end

-- type creator = 
-- & ( <V, E>(v: V, e: nil)-> result<V, E> )
-- & ( <V, E>(v: nil, e: E) -> result<V, E> )
-- & ( <E>() -> result<nil, E> ) 
return (create :: any)
    :: (<V, E>(v: V, e: nil)-> result<V, E>)
    & ( <V, E>(v: nil, e: E) -> result<V, E> )
    & ( <E>() -> result<nil, E> ) 
