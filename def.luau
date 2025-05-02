--!strict
local newresult = require('@pls/result')
local e = require('@test/module')
print(e.name)
newresult(nil, "eee")
local w = setmetatable({}, {__mode = 'v'})
local writer = pls.io.file_writer("heeelllo", true)
w.writer = writer
w.writer:print("yooooooooo")
local e = pls.io.stdin:try_scan():and_then(
    function(e)
        return e:reverse()
    end)
writer = nil :: never
pls.gc.collect()
--local writer2 = pls.io.file_writer("heeelllo", true)
w.writer:print("bro what")
w.writer:print("a", 'b', 1, 2, 3.2, '\n')
    :writeu8(2)
    :writef64(123.123)
    :write("\n\n\n\nboom")
    :writeu8(12)
    :writei8(12)
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
