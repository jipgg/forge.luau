--!strict
local e = require('@test/module')
print(e.name)
local writer = pls.io.file_writer("heeelllo", true)
writer:print("a", 'b', 1, 2, 3.2, '\n')
    :writeu8(2)
    :writef64(12.34)
    :writef64(12.34)
    :writef64(12.34)
    :writef64(12.34)
    :writef64(12.34)
    :write("\n\n\n\nboom")
    :writeu8(12)
    :writei8(12)
    :style{fg='red'}:print("eeeee"):style{fg='reset'}
local e = pls.result("abc", nil)
print(pls.fs.current_path())
-- local result = pls.monad.result
-- local path = pls.filesystem.path
--
-- local function something(b: boolean)
--     if b then return result(nil, "this is an error")
--     else return result(pls.filesystem.current_path(), nil) end
-- end
-- print(something(true):and_then(function(some_path)
--     return some_path:expect_directory():match(
--         function(dir) return "is a directory" end,
--         function(other)
--             return other:expect_file():match(
--                 function(file) return "is a file" end,
--                 function(symlink) return "is a symlink" end)
--         end)
-- end))
