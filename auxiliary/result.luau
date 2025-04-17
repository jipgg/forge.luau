local meta = {} :: meta<any, any>
meta.__index = meta

function meta:has_value()
    return self._has_value
end
function meta:has_error()
    return not self._has_value
end
function meta:and_then(fn)
    return if self._has_value then fn(self._data) else self
end
function meta:or_else(fn)
    return if self._has_value then self else fn(self._data)
end
function meta:value()
    return if not self._has_value then error("bad result access") else self._data
end
function meta:error()
    return if self._has_value then error("bad result access") else self._data
end
function meta:value_or(val)
    return if self._has_value then self._data else val
end
function meta:error_or(err)
    return if self._has_value then err else self._data
end
function meta:match<Ret>(valfn, errfn)
    return if self._has_value then valfn(self._data) else errfn(self._data)
end
type meta<Ty, Err> = {
    __index: meta<Ty, Err>,
    has_value: (result<Ty, Err>) -> boolean,
    has_error: (result<Ty, Err>) -> boolean,
    and_then: (result<Ty, Err>, fn: (Ty)->result<Ty, Err>) -> result<Ty, Err>,
    or_else: (result<Ty, Err>, fn: (Ty)->result<Ty, Err>) -> result<Ty, Err>,
    match: <Ret>(result<Ty, Err>, valfn: (Ty)->Ret, errfn: (Err)->Ret) -> Ret,
    error: (result<Ty, Err>) -> Err,
    value: (result<Ty, Err>) -> Ty,
    error_or: (result<Ty, Err>, err: Err) -> Err,
    value_or: (result<Ty, Err>, val: Ty) -> Ty,
}
export type result<Ty = nil, Err = string> = typeof(setmetatable(
    {} :: {
    _data: Ty | Err,
    _has_value: boolean,
    },
    {} :: meta<Ty, Err>
))
local result = {}
function result.value<Ty, Err>(v: Ty): result<Ty, Err>
    return setmetatable({
        _data = v,
        _has_value = true,
    }, meta)
end
function result.error<Ty, Err>(err: Err): result<Ty, Err>
    return setmetatable({
        _data = err,
        _has_value = false,
    }, meta)
end
function result.transform<From, To, Err>(v: result<From, Err>, fn: (From)->To): result<To, Err>
    return if v._has_value
    then result.value(fn(v._data :: From))
    else result.error(v._data :: Err)
end
function result.transform_error<Ty, From, To>(v: result<Ty, From>, fn: (From)->To): result<Ty, To>
    return if v._has_value
    then result.value(v._data :: Ty)
    else result.error(fn(v._data :: From))
end
function result.wrap<Ty, Args...>(fn: (Args...) -> Ty): result<Ty, string>
    local success, data = pcall(fn)
    return if success
    then result.value(data)
    else result.error(data :: any)
end
return result
