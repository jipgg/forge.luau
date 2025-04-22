--!strict
--module.yo()
local abc = {a = 123, b = "hello", c = true, d = {e = 1, f=2, g=3, h=4}, g = {1, 2, 3, 4, 5}}
local to_json = blt.json.to_json(abc)
local parsed: typeof(abc) = blt.json.parse(to_json)
local path = blt.fs.current_path() / "test_file"
blt.fio.open_writer(path, true):close_after(function(writer)
    writer:write(buffer.fromstring("abcdefg\n"))
    writer:write_string("eeeeeeeeeeeee\n")
    writer ..= "yoyoyoyoyoyoyoyy\n"
end)
local file_data = blt.fio.read_to_end(path)
print(buffer.tostring(file_data))

local src = blt.fs.current_path() / "src"
for sub_path in blt.fs.directory_iterator(src) do
    local canonical = blt.fs.canonical(sub_path)
    print(`{canonical}; {canonical:generic_string()}`)
end
blt.proc.system("echo yo man")
blt.proc.system("echo")
print(table.concat(blt.proc.args(), ";"))

for arg in blt.proc.arg_iterator() do
    print(`arg = {arg}`)
end
local path = blt.fs.path(".")

@native
local function add(a: number, b: number): number
    return a + b
end
print(add(1, 2))

print(to_json, parsed.b, parsed.g[4])

print(typeof(blt.fs.current_path()))

print()
