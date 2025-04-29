--!strict
type stack<T> = {
    empty: <Stack>(Stack) -> boolean,
}
local writer = pls.io.file_writer("heeelllo")
writer:print("a", 'b', 1, 2, 3.2, '\n')
    :writei8(1)
    :writeu8(2)
    :writef64(12.34)
    :writef64(12.34)
    :writef64(12.34)
    :writef64(12.34)
    :writef64(12.34)
    :close()
local e = pls.result("abx", nil)
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
