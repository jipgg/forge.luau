local module = require('./some_module')
local fio = builtin.fileio
local fs = builtin.filesystem
print(module)
--module.yo()
local abc = {a = 123, b = "hello", c = true, d = {e = 1, f=2, g=3, h=4}, g = {1, 2, 3, 4, 5}}
local e = builtin.json.to_json(abc)
local parsed: typeof(abc) = builtin.json.parse(e)
local test_file = builtin.filesystem.path("test_file")
local test_writer = fio.open_append_writer(test_file)
test_writer += "yoyoyoyoy\n"
test_writer:close()
print(fio.read_all(test_file))


local path = fs.current_path() / "src"
local aee = fs.temp_directory_path()
for sub_path in fs.directory_iterator(path) do
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

