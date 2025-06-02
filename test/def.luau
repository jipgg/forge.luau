
local args = {...}
for i, v in args do
    print(`args: {i} = {v}`)
end
print(typeof(forge.io.stdin))
print(_VERSION)
print(_G)
local r = forge.io.filereader("test/def.luau")
for line in r:lines() do
    print(line)
end
forge.fs.path("eeee")
forge.io.stdout("hello")
for i, v in forge do
    print(`{i} = {v}`)
end

print(forge.fs.path("abc").canonical)
-- local stack = require('@pls/stack')
-- print("yoo")
-- local path = fs.path("hello.luau")
-- path.ext = "lul"
-- path.stem = "eeaeawdawdaw"
-- print(path.type)
-- print(typeof(path))
-- print(path.ext, path.name, path.stem)
-- path.dir = fs.path('../')
-- local fw = io.filewriter("aec")
-- fw:close()
-- fs.copy("aec", "aec2", 'overwrite existing')
-- os.execute("echo hello")
