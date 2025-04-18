local fs = builtin.filesystem
local fio = builtin.fileio
--module.yo()
local abc = {a = 123, b = "hello", c = true, d = {e = 1, f=2, g=3, h=4}, g = {1, 2, 3, 4, 5}}
local e = builtin.json.to_json(abc)
local parsed: typeof(abc) = builtin.json.parse(e)
local path = builtin.filesystem.current_path() / "test_file"
assert(fio.open_writer(path, true):close_after(function(writer)
    writer ..= "yoyoyoyoyoyoyoyy\n"
end))
print(builtin.fileio.read_all(path))

local src = fs.current_path() / "src"
local aee = fs.temp_directory_path()
for sub_path in fs.directory_iterator(src) do
    local canonical = fs.canonical(sub_path)
    print(`{canonical}; {canonical:generic_string()}`)
end
builtin.process.system("echo yo man")

for arg in builtin.process.arg_iterator() do
    print(`arg = {arg}`)
end

@native
local function add(a: number, b: number): number
    return a + b
end
print(add(1, 2))

print(e, parsed.b, parsed.g[4])

print(typeof(fs.current_path()))

print()
