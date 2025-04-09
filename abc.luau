local abc = {a = 123, b = "hello", c = true, d = {e = 1, f=2, g=3, h=4}, g = {1, 2, 3, 4, 5}}
local fs = xt.filesystem
local fio = xt.fileio
local e = xt.json.to_json(abc)
local parsed: typeof(abc) = xt.json.parse(e)
local test_file = fs.path("test_file")
fio.write_file(test_file, "yoyoyoyo")
print(fio.read_file(test_file))
fs.remove(test_file)

print(e, parsed.b, parsed.g[4])

print(typeof(fs.current_path()))
