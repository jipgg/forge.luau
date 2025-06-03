
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
type query<T> = {
    where: (pred: (T) -> boolean) -> query<T>,
    iterator: (() -> T?),
    table: {T},
}
local iter = {
    filter = function<T>(iter: ()->T?, pred: (T)->boolean): ()->T?
        return function()
            for e in iter do
                if pred(e) then return e
                else continue end
            end
            return nil
        end
    end,
}
local path = forge.fs.path("eeee")
forge.io.stdout(`{path}`)
if path.isrelative then
    path = path.absolute
end
forge.io.stdout(`{path}`)
forge.io.stdout("hello")
for i, v in forge do
    print(`{i} = {v}`)
end
local cwd = forge.fs.path.current()
local pred = function(p: path)
    return p.type == "directory"
end
local cor = coroutine.create(function(...)
    for p in cwd:children() do
        coroutine.yield(p)
    end
end)
local _, v: path = coroutine.resume(cor)
while v do
    _, v = coroutine.resume(cor)
    print(v.filename, v.type, v.parent.canonical.generic)
end
local b, p = coroutine.resume(cor)
print(forge.fs.path.current())

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
