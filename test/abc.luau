local fs = builtin.filesystem
local con = builtin.console
local println = con.println
con.print("eee")
local proc = builtin.process
local abc = {a = 123, b = "hello", c = true, d = {e = 1, f=2, g=3, h=4}, g = {1, 2, 3, 4, 5}}
plus.
local to_json = builtin.json.to_json(abc)
local parsed: typeof(abc) = builtin.json.parse(to_json)
local path = fs.current_path() / "test_file"
builtin.file.open_writer(path, true):close_after(function(writer)
    writer:write(buffer.fromstring("abcdefg\n"))
    writer:println("eeeeeeeeeeeee")
    writer ..= "yoyoyoyoyoyoyoyy\n"
end)
local writer = builtin.file.open_writer(path)
local file_data = builtin.file.read_to_end(path)
print(buffer.tostring(file_data))

local src = fs.current_path() / "src"
for sub_path in fs.directory_iterator(src) do
    local canonical = fs.canonical(sub_path)
    print(`{canonical}; {canonical:generic_string()}`)
end
builtin.process.system("echo yo man")
print(table.concat(proc.args(), ";"))

for arg in proc.arg_iterator() do
    print(`arg = {arg}`)
end

@native
local function add(a: number, b: number): number
    return a + b
end
print(add(1, 2))

print(to_json, parsed.b, parsed.g[4])

print(typeof(fs.current_path()))

print()
