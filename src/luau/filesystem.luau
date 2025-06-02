type copyoptions = "recursive" | "update existing" | "skip existing" | "create symlinks" | "copy symlinks" | "overwrite existing" | "directories only" | "create hard links" | "none"
type iterator<T, U = nil> = () -> (T?, U)
type filetype = "directory" | "symlink" | "file" | "unknown"
type anypath = path | string
type path_t = {
    extension: string,
    filename: string,
    parent: path,
    stem: string,
    filetype: string,
    string: (path) -> string,
    isabsolute: (path) -> string,
    isrelative: (path) -> string,
    child: (path, subpath: path | string) -> path,
    children: (path, recursive: boolean?) -> (() -> path?),
    clone: (path) -> path,
}
type path_mt = {
    __div: (path, other: path | string) -> path,
    __tostring: (path) -> string,
}
export type path = typeof(setmetatable({}::path_mt, {}::path_t))
type filesystem = {
    rename: (from: anypath, to: anypath) -> (),
    remove: (path: anypath, all: boolean?) -> boolean,
    exists: (path: anypath) -> boolean,
    equivalent: (a: anypath, b: anypath) -> boolean,
    newdir: (path: anypath, recursive: boolean?) -> boolean,
    subpaths: (dir: anypath, recursive: boolean?) -> iterator<path>,
    currdir: () -> path,
    type: (path: anypath) -> filetype,
    tmpdir: () -> path,
    canonical: (path: anypath, weakly: boolean?) -> path,
    absolute: (path: anypath) -> path,
    copy: (from: anypath, to: anypath, opts: copyoptions?) -> (),
    newsym: (to: anypath, new_symlink: anypath) -> (),
    newfile: (path: anypath, text: string?, force: boolean?) -> (),
    getenv: (var: string) -> path?,
    readsym: (symlink: anypath) -> path,
    homedir: () -> path,
    path: ((path: string) -> path),
}

return {} :: filesystem
