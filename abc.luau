local abc = {a = 123, b = "hello", c = true, d = {e = 1, f=2, g=3, h=4}, g = {1, 2, 3, 4, 5}}
local e = json.to_json(abc)
local parsed: typeof(abc) = json.parse(e)

print(e, parsed.b, parsed.g[4])
local cwd = filesystem.current_path()
print(cwd)

