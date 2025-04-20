--module.yo()
local abc = {a = 123, b = "hello", c = true, d = {e = 1, f=2, g=3, h=4}, g = {1, 2, 3, 4, 5}}
local e = builtin.json.to_json(abc)
local parsed: typeof(abc) = builtin.json.parse(e)
local path = builtin.fs.current_path() / "test_file"
builtin.fio.open_writer(path, true):close_after(function(writer)
    writer:write(buffer.fromstring("abcdefg\n"))
    writer:write_string("eeeeeeeeeeeee\n")
    writer ..= "yoyoyoyoyoyoyoyy\n"
end)
local file_data = builtin.fio.read_to_end(path)
print(buffer.tostring(file_data))
local buf2 = builtin.cio.read(12)
print(`{buffer.len(buf2)}: {buffer.tostring(buf2)}`)

local src = builtin.fs.current_path() / "src"
for sub_path in builtin.fs.directory_iterator(src) do
    local canonical = builtin.fs.canonical(sub_path)
    print(`{canonical}; {canonical:generic_string()}`)
end
builtin.proc.system("echo yo man")

for arg in builtin.proc.arg_iterator() do
    print(`arg = {arg}`)
end

@native
local function add(a: number, b: number): number
    return a + b
end
print(add(1, 2))

print(e, parsed.b, parsed.g[4])

print(typeof(builtin.fs.current_path()))

print()
