type copyoptions = "recursive" | "update existing" | "skip existing" | "create symlinks" | "copy symlinks" | "overwrite existing" | "directories only" | "create hard links" | "none"
type filetype = "directory" | "symlink" | "file" | "unknown"
type anypath = path | string
type iterator<T, U = nil> = () -> (T?, U)
declare class path
    ext: string
    name: string
    dir: path
    stem: string
    type: filetype
    function string(self): string
    function generic(self): string
    function isabsolute(self): boolean
    function isrelative(self): boolean
    function subpath(self, subpath: anypath): path
    function clone(self): path
    function subpaths(self, recursive: boolean?): iterator<path>
    function __div(self, other: anypath): path
    function __tostring(self): string
end
declare class reader
    function read(self, count: number?): string
    function lines(self): iterator<string>
    function readu8(self): number
    function readi8(self): number
    function readu16(self): number
    function readi16(self): number
    function readu32(self): number
    function readi32(self): number
    function readf32(self): number
    function readf64(self): number
    function flush(self): reader
    function __call(self): string
end
declare class writer
    write: <Writer>(self: Writer, data: string | buffer) -> Writer
    writeu8: <Writer>(self: Writer, v: number) -> Writer
    writei8: <Writer>(self: Writer, v: number) -> Writer
    writeu16: <Writer>(self: Writer, v: number) -> Writer
    writei16: <Writer>(self: Writer, v: number) -> Writer
    writeu32: <Writer>(self: Writer, v: number) -> Writer
    writei32: <Writer>(self: Writer, v: number) -> Writer
    writef32: <Writer>(self: Writer, v: number) -> Writer
    writef64: <Writer>(self: Writer, v: number) -> Writer
    flush: <Writer>(self: Writer) -> Writer
    seek: <Writer>(self: Writer, pos: number, base: "beg" | "cur" | "end" | nil) -> Writer
    function tell(self): number
    __call: <T...>(self: writer, T...) -> writer
end
declare class filereader extends reader
    function lines(self, autoclose: boolean?): (() -> string?)
    isopen: boolean
    path: path
    close: ((self: filereader, before: ((reader) -> ())?) -> ())
    function __call(self): string
end
declare class filewriter extends writer
    path: path
    isopen: boolean
    close: ((self: filewriter, before: ((writer) -> ())?) -> ())
    __call: <T...>(self: filewriter, T...) -> filewriter
end
type fs = {
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
    path: (path: string) -> path,
}
type io = {
    stdin: reader,
    stdout: writer,
    stderr: writer,
    filewriter: (file: anypath, append: boolean?) -> filewriter,
    filereader: (file: anypath) -> filereader,
}
type proc = {
    shell: (command: string) -> number,
    args: () -> iterator<string>,
    sleep: (seconds: number) -> (), 
}
declare io: io
declare proc: proc
declare fs: fs
