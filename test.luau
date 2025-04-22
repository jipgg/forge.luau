local home = blt.fs.home_path()
print(home, home:filename(), home:extension())

for path in home:directory_iterator(true) do
    print(path)
end
