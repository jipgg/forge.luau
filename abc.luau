local aux = require('@auxiliary')
local result = require('@auxiliary/result')
local fs = builtin.filesystem
local fio = builtin.fileio
local mf = aux.multicast{
    functions = {
        function(a: number, b: string)
            return 1
        end,
        function(e: number, f: string)
            return ""
        end
    }
}
local fn = function(a: number, e: string)
    return 3
end
local r = result.wrap(function(a: number)
    error("gggg")
    return 1
end)
if r:has_error() then
    print(`err result is {r:error()}`)
else 
    print(`result is {r:value()}`)
end
result.transform(r, function(v)
    return tostring(v)
end)
print(`mf count {mf:size()}`)
mf += fn
print(`mf count {#mf}`)
mf(1, "eee")
mf -= fn
print(`mf count {#mf}`)
--module.yo()
local abc = {a = 123, b = "hello", c = true, d = {e = 1, f=2, g=3, h=4}, g = {1, 2, 3, 4, 5}}
local e = builtin.json.to_json(abc)
local parsed: typeof(abc) = builtin.json.parse(e)
local path = builtin.filesystem.current_path() / "test_file"
assert(fio.open_writer(path):scope(function(writer)
    writer ..= "yoyoyoyoyoyoyoyy\n"
end))
print(builtin.fileio.read_all(path))

local src = fs.current_path() / "src"
local aee = fs.temp_directory_path()
for sub_path in fs.directory_iterator(src) do
    local canonical = fs.canonical(sub_path)
    print(`{canonical}; {canonical:generic_string()}`)
end

@native
local function add(a: number, b: number): number
    return a + b
end
print(add(1, 2))

print(e, parsed.b, parsed.g[4])

print(typeof(fs.current_path()))

print()

