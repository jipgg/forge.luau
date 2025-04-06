local Some_module = require('./some_module')
print(fs.create_directories("yo/a/b/c/d/e/f/g/h/i/j"))
fs.copy("yo", "yo2", 'recursive')
print(fs.remove_all("yo"))
print(fs.remove_all("yo2"))

print(fs.temp_directory_path())
print(fs.weakly_canonical('.'))
print(Some_module.abc)
