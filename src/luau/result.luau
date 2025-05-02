export type result<V, E> = {
    _ok: boolean,
    _v: V | E,
    and_then: <Result, T>(Result, (V) -> Result) -> Result,
    unpack: <Result>(Result) -> (V?, E?),
    value: <Result>(Result) -> V | never,
    error: <Result>(Result) -> E,
    has_value: <Result>(Result) -> boolean,
    has_error: <Result>(Result) -> boolean,
    value_or: <Result>(Result, V) -> V, 
    or_else: <Result>(Result, (E) -> Result) -> Result,
    error_or: <Result>(Result, E) -> E, 
    match: <Result, T>(Result, (V) -> T, (E) -> T) -> T,
    switch: <Result>(Result, (V) -> (), (E) -> ()) -> (),
}
local meta = {} :: anyresult
(meta :: any).__index = meta
function meta:and_then(fn)
    return self._ok and fn(self._v) or self
end
function meta:or_else(fn)
    return not self._ok and fn(self._v) or self
end
function meta:value()
    return self._ok and self._v or error("bad access")
end
function meta:error()
    return not self._ok and self._v or error("bad access")
end
function meta:has_value()
    return self._ok
end
function meta:has_error()
    return not self._ok
end
function meta:value_or(v)
    return self._ok and self._v or v
end
function meta:error_or(v)
    return not self._ok and self._v or v
end
function meta:match(v, e)
    return self._ok and v(self._v) or e(self._v)
end
function meta:switch(v, e)
    return self._ok and v(self._v) or e(self._v)
end

function meta:unpack()
    if self._ok then
        return self._v, nil;
    else
        return nil, self._v;
    end
end
export type anyresult = {
    _ok: boolean,
    _v: any,
    and_then: (anyresult, (any)->anyresult) -> anyresult,
    unpack: (anyresult) -> (any, any),
    value: (anyresult) -> any,
    error: (anyresult) -> any,
    has_value: (anyresult) -> boolean,
    has_error: (anyresult) -> boolean,
    value_or: (anyresult, any) -> any, 
    or_else: (anyresult, (any) -> anyresult) -> anyresult,
    error_or: (anyresult, any) -> any, 
    match: (anyresult, (any)->any, (any)->any) -> any,
    switch: (anyresult, (any)->(), (any)->()) -> (),
}
type creator = 
& ( <V, E>(v: V, e: nil)-> result<V, E> )
& ( <V, E>(v: nil, e: E) -> result<V, E> )
& ( <E>() -> result<nil, E> )

local function create<Result>(v, e): Result
    return setmetatable({
        _ok = v ~= nil,
        _v = v and v or e,
    }, meta) :: any
end

return (create :: any) :: creator
