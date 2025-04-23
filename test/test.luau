local fs = builtin.filesystem.home_path()
local home = builtin.filesystem.home_path()
local id = "id1"
builtin.process.bind_to_exit(function()
    print("yoyoyoyyoyoyo")
end, id)
print(builtin.process.unbind_from_exit(id))
print(home, home:filename(), home:extension())
