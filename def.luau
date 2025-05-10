local result = require('@pls/result')
local client = http.client("http://httpbin.org")
fs.path("eeeeeeeeeeeee"):subpath("eeee"):subpath("eeeeee"):generic()
print("Performing GET request...")
local status, body = client:get("/get")
if status then
    io.cout(`GET: {status}, {body}`)
else
    io.cout("GET Request Error:", body)
end
fs.path("eeee")
local e = fs.path("eee") / "eeee"

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
