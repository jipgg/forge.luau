local some_module = require('./some_module')

for entry in Fs.iterator(".", false) do
    print(`entry is {entry}`)
end

print("hello")
print(some_module.abc)
