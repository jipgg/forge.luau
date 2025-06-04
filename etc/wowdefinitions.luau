type copyoptions = "recursive" | "update existing" | "skip existing" | "create symlinks" | "copy symlinks" | "overwrite existing" | "directories only" | "create hard links" | "none"
type filetype = "directory" | "symlink" | "file" | "unknown"
type anypath = path | string
type iterator<T, U = nil> = () -> (T?, U)
export type path = {
    read extension: string,
    read filename: string,
    read parent: path,
    read stem: string,
    read filetype: filetype,
    read absolute: path,
    read canonical: path,
    read string: string,
    read generic: string,
    read native: string,
    read isrelative: boolean,
    read isabsolute: boolean,
    child: (self: path, subpath: anypath) -> path,
    clone: (self: path) -> path,
    children: (self: path, recursive: boolean?) -> iterator<path>
}
export type reader = {
    read: <Reader>(self: Reader, count: number?) -> string,
    scan: <Reader>(self: Reader) -> string,
    lines: <Reader>(self: Reader) -> (() -> string?),
    readu8: <Reader>(self: Reader) -> number,
    readi8: <Reader>(self: Reader) -> number,
    readu16: <Reader>(self: Reader) -> number,
    readi16: <Reader>(self: Reader) -> number,
    readu32: <Reader>(self: Reader) -> number,
    readi32: <Reader>(self: Reader) -> number,
    readf32: <Reader>(self: Reader) -> number,
    readf64: <Reader>(self: Reader) -> number,
    flush: <Reader>(self: Reader) -> Reader,
}
export type writer = {
    write: <Writer>(self: Writer, data: string | buffer) -> Writer,
    writeu8: <Writer>(self: Writer, v: number) -> Writer,
    writei8: <Writer>(self: Writer, v: number) -> Writer,
    writeu16: <Writer>(self: Writer, v: number) -> Writer,
    writei16: <Writer>(self: Writer, v: number) -> Writer,
    writeu32: <Writer>(self: Writer, v: number) -> Writer,
    writei32: <Writer>(self: Writer, v: number) -> Writer,
    writef32: <Writer>(self: Writer, v: number) -> Writer,
    writef64: <Writer>(self: Writer, v: number) -> Writer,
    flush: <Writer>(self: Writer) -> Writer,
    seek: <Writer>(self: Writer, pos: number, base: "beg" | "cur" | "end" | nil) -> Writer,
    tell: <Writer>(self: Writer) -> number,
}
export type filereader = reader & {
    lines: (self: filereader, autoclose: boolean?) -> (() -> string?),
    read isopen: boolean,
    close: ((self: filereader, before: ((reader) -> ())?) -> ()),
}
export type filewriter = writer & {
    read isopen: boolean,
    close: ((self: filewriter, before: ((writer) -> ())?) -> ()),
}
export type httpresponse = {
    read status: number,
    read body: string,
    read location: string,
    read version: string,
    read reason: string,
    headers: (self: httpresponse) -> {[string]: string},
}
export type httpclient = {
    read host: string,
    read isvalid: boolean,
    read port: number,
    --- is write only
    keepalive: boolean,
    encodeurl: boolean,
    connectiontimeout: number,
    maxtimeout: number,
    readtimeout: number,
    writetimeout: number,
    stop: (self: httpclient) -> (),
    get: (self: httpclient, path: string) -> (httpresponse?, string),
    --post: (self: httpclient, path: string, body: unknown) -> (httpresponse?, string),
}
type filesystemlibrary = {
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
type iolibrary = {
    stdin: reader,
    stdout: writer,
    stderr: writer,
    readfile: (file: anypath) -> string?,
    writefile: (file: anypath, contents: string) -> boolean,
    filewriter: ((file: anypath, append: boolean?) -> filewriter),
    filereader: ((file: anypath) -> filereader),
}
type httplibrary = {
    get: (url: string) -> (httpresponse?, string),
    post: (url: string, args: unknown) -> (httpresponse?, string),
    client: ((host: string) -> httpclient),
}
type processlibrary = {
    system: (command: string) -> number,
    sleep: (seconds: number) -> (), 
}
type jsonlibrary = {
    tostring: <T>(t: T) -> string,
    parse: <T>(src: string) -> T,
}
declare collectgarbage: (('collect') -> ()) & (('count') -> number)
declare wow: {
    fs: filesystemlibrary,
    proc: processlibrary,
    io: iolibrary,
    http: httplibrary,
    json: jsonlibrary,
}
