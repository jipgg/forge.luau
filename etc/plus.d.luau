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
    function scan(self): string
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
    write: <anywriter>(self: anywriter, data: string | buffer) -> anywriter
    writeu8: <anywriter>(self: anywriter, v: number) -> anywriter
    writei8: <anywriter>(self: anywriter, v: number) -> anywriter
    writeu16: <anywriter>(self: anywriter, v: number) -> anywriter
    writei16: <anywriter>(self: anywriter, v: number) -> anywriter
    writeu32: <anywriter>(self: anywriter, v: number) -> anywriter
    writei32: <anywriter>(self: anywriter, v: number) -> anywriter
    writef32: <anywriter>(self: anywriter, v: number) -> anywriter
    writef64: <anywriter>(self: anywriter, v: number) -> anywriter
    flush: <anywriter>(self: anywriter) -> anywriter
    seek: <anywriter>(self: anywriter, pos: number, base: "beg" | "cur" | "end" | nil) -> anywriter
    function tell(self): number
    __call: <T...>(self: writer, T...) -> writer
end
declare class filereader extends reader
    function lines(self, autoclose: boolean?): (() -> string?)
    isopen: boolean
    close: ((self: filereader, before: ((reader) -> ())?) -> ())
    function __call(self): string
end
declare class filewriter extends writer
    isopen: boolean
    close: ((self: filewriter, before: ((writer) -> ())?) -> ())
    __call: <T...>(self: filewriter, T...) -> filewriter
end
declare class httpclient
    function get(self, path: string): (number?, string?)
    function post(self, path: string, body: string): (number?, string?)
end
type fslib = {
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
type iolib = {
    stdin: reader,
    stdout: writer,
    stderr: writer,
    readfile: (file: anypath) -> string?,
    writefile: (file: anypath, contents: string) -> boolean,
    filewriter: ((file: anypath, append: boolean?) -> filewriter),
    filereader: ((file: anypath) -> filereader),
}
type httplib = {
    get: (host: string, path: string) -> (number?, string),
    post: (host: string, path: string, body: string) -> (number?, string),
    client: ((host: string) -> httpclient),
}
type oslib = {
    is: ('linux' | 'windows' | 'macos') -> boolean,
    execute: (command: string) -> number,
    getenv: (env: string) -> string?,
    args: () -> iterator<string>,
    sleep: (seconds: number) -> (), 
}
type jsonlib = {
    encode: <T>(t: T) -> string,
    decode: <T>(src: string) -> T,
}
declare io: iolib
declare os: typeof(os) & oslib
declare fs: fslib
declare http: httplib
declare json: jsonlib
declare collectgarbage: (('collect') -> ()) & (('count') -> number)
