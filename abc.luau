print(plus.garbage.count())
print(...)
plus.io.stderr:print("helllo"):ln()
plus.io.stdout:println("eee"):println("eee"):print("eeeee"):print("aaaa")
plus.io.stderr:print("eeeeea")
for i, v in plus.filesystem.current_path():child("src"):descendants() do
    print(`{i} = {v:generic_string()}`)
end


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
