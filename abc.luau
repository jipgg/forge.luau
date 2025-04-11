local module = require('./some_module')
print(module)
--module.yo()
local abc = {a = 123, b = "hello", c = true, d = {e = 1, f=2, g=3, h=4}, g = {1, 2, 3, 4, 5}}
local e = json.to_json(abc)
local parsed: typeof(abc) = json.parse(e)
local test_file = filesystem.path("test_file")
fileio.write_file(test_file, "yoyoyoyo")
print(fileio.read_file(test_file))
filesystem.remove(test_file)


local path = filesystem.current_path() / "src"
local aee = filesystem.temp_directory_path()
for sub_path in filesystem.directory_iterator(path) do
    local canonical = filesystem.canonical(sub_path)
    print(`{canonical}; {canonical:generic_string()}`)
end

@native
local function add(a: number, b: number): number
    return a + b
end
print(add(1, 2))

print(e, parsed.b, parsed.g[4])

print(typeof(filesystem.current_path()))

print()
