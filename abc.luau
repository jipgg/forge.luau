print(plus.garbage.count())
print(...)
plus.io.stderr:print("helllo"):ln()
plus.io.stdout:println("eee"):println("eee"):print("eeeee"):print("aaaa")
plus.io.stderr:print("eeeeea")
print(`garbage: {plus.garbage.count()}kb`)
for child in plus.filesystem.current_path():child("-cmake"):iterate_descendants() do
    print(child:string())
end
print(`garbage: {plus.garbage.count()}kb`)
plus.garbage.collect()
print(`garbage: {plus.garbage.count()}kb`)


plus.io.filewriter("abc", true):println("im chanining"):close_after(function(self)
    print("closing")
end)
plus.io.filewriter("abc", true):close_after(function(self: __writer)
    self:println("helllo"):print("byteeee"):ln()
end)

local cwd = plus.filesystem.current_path()
print(cwd)
local m = require("@test")
print(m.name)
print(plus.garbage.count())
