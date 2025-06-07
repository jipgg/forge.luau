export type copyoptions = "recursive" | "update existing" | "skip existing" | "create symlinks" | "copy symlinks" | "overwrite existing" | "directories only" | "create hard links" | "none"
export type filetype = "directory" | "symlink" | "file" | "unknown"
type path_u = path | string
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
    child: (self: path, subpath: path_u) -> path,
    clone: (self: path) -> path,
    children: (self: path, recursive: boolean?) -> (() -> path?),
}
type filesystem = {
    rename: (from: path_u, to: path_u) -> (),
    remove: (path: path_u, all: boolean?) -> boolean,
    exists: (path: path_u) -> boolean,
    equivalent: (a: path_u, b: path_u) -> boolean,
    newdir: (path: path_u, recursive: boolean?) -> boolean,
    subpaths: (dir: path_u, recursive: boolean?) -> (() -> path?),
    currdir: () -> path,
    type: (path: path_u) -> filetype,
    tmpdir: () -> path,
    canonical: (path: path_u, weakly: boolean?) -> path,
    absolute: (path: path_u) -> path,
    copy: (from: path_u, to: path_u, opts: copyoptions?) -> (),
    newsym: (to: path_u, new_symlink: path_u) -> (),
    newfile: (path: path_u, text: string?, force: boolean?) -> (),
    getenv: (var: string) -> path?,
    readsym: (symlink: path_u) -> path,
    homedir: () -> path,
    path: ((path: string) -> path),
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

type lookup = setmetatable<{}, {
    __index: (self: lookup, key: string) -> string,
}>
export type httpresponse = {
    read status: number,
    read body: string,
    read location: string,
    read version: string,
    read reason: string,
    read headers: lookup,
    getheaders: (self: httpresponse) -> {[string]: string},
    getheadervalue: (self: httpresponse, key: string, default: string?) -> string,
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
type urlinfo = {
    scheme: string,
    host: string,
    port: number,
    path: string,
}
type http = {
    urlinfo: (url: string) -> urlinfo,
    get: (url: string) -> (httpresponse?, string),
    post: (url: string, args: unknown) -> (httpresponse?, string),
    client: ((host: string) -> httpclient),
}
type io = {
    stdin: reader,
    stdout: writer,
    stderr: writer,
    readfile: (file: path_u) -> string?,
    writefile: (file: path_u, contents: string) -> boolean,
    filewriter: ((file: path_u, append: boolean?) -> filewriter),
    filereader: ((file: path_u) -> filereader),
}
type process = {
    system: (command: string) -> number,
    sleep: (seconds: number) -> (), 
}
type json = {
    tostring: <T>(t: T) -> string,
    parse: <T>(src: string) -> T,
}
type wow = {
    fs: filesystem,
    proc: process,
    io: io,
    http: http,
    json: json,
}
type collectgarbage = (('collect') -> ()) & (('count') -> number)

declare wow: wow
declare collectgarbage: collectgarbage
