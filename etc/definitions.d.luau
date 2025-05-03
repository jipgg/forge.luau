type copy_options = "recursive" | "update existing" | "skip existing" | "create symlinks" | "copy symlinks" | "overwrite existing" | "directories only" | "create hard links" | "none"
type file_type = "directory" | "symlink" | "file"
type as_path = {as_path: (any) -> path}
type anypath = path | string | as_path
type iterator<T, U = nil> = () -> (T?, U)
export type path_type = {
    string: <Path>(Path) -> string,
    generic_string: <Path>(Path) -> string,
    extension: <Path>(Path) -> Path,
    has_extension: <Path>(Path) -> boolean,
    replace_extension: <Path>(Path) -> Path,
    parent: <Path>(Path) -> Path,
    child: <Path>(Path, name: Path | string) -> Path,
    is_absolute: <Path>(Path) -> boolean,
    is_relative: <Path>(Path) -> boolean,
    filename: <Path>(Path) -> Path,
    has_filename: <Path>(Path) -> boolean,
    replace_filename: <Path>(Path, filename: Path | string) -> Path,
    remove_filename: <Path>(Path) -> Path,
    remove_extension: <Path>(Path) -> Path,
    is_directory: <Path>(Path) -> boolean,
    is_file: <Path>(Path) -> boolean,
    is_symlink: <Path>(Path) -> boolean,
    descendant_iterator: <T>(T) -> iterator<T>,
    child_iterator: <T>(T) -> iterator<T>,
    descendants: <T>(T) -> {T},
    children: <T>(T) -> {T},
    clone: <T>(T) -> T,
}
export type path_meta = {
    __div: <T>(T, anypath) -> T,
    __concat: <T>(T, nil) -> T,
}
export type path = typeof(setmetatable({} :: path_type, {} :: path_meta))
export type reader = {
    scan: <T>(T) -> string,
    getline: <T>(T, line: string) -> boolean,
    line_iterator: <T>(T) -> iterator<string>,
    readu8: <T>(T) -> number,
    readi8: <T>(T) -> number,
    readi16: <T>(T) -> number,
    readu16: <T>(T, val: number) -> number,
    readi32: <T>(T, val: number) -> number,
    readu32: <T>(T, val: number) -> number,
    readf32: <T>(T, val: number) -> number,
    readf64: <T>(T, val: number) -> number,
    flush: <T>(T) -> T,
    eof: <T>(T) -> boolean,
}
type console_color = 'reset' | 'black' | 'red' | 'green' | 'yellow' | 'blue' | 'magenta' | 'cyan' | 'white' | 'default'
type console_style = {
    fg: console_color?,
    bold: boolean?,
    dim: boolean?,
    underlined: boolean?,
    blinking: boolean?,
    hidden: boolean?,
    bg: console_color?,
}
export type writer = {
    print: <T, U...>(self: T, U...) -> T,
    write: <T, U...>(self: T, U...) -> T,
    writestring: <T>(T, str: string) -> T,
    writebuffer: <T>(T, buf: buffer) -> T,
    writeu8: <T>(T, val: number) -> T,
    writei8: <T>(T, val: number) -> T,
    writei16: <T>(T, val: number) -> T,
    writeu16: <T>(T, val: number) -> T,
    writei32: <T>(T, val: number) -> T,
    writeu32: <T>(T, val: number) -> T,
    writef32: <T>(T, val: number) -> T,
    writef64: <T>(T, val: number) -> T,
    flush: <T>(T) -> T,
    eof: <T>(T) -> boolean,
    good: <T>(T) -> boolean,
    bad: <T>(T) -> boolean,
    seekp: <T>(T, pos: number) -> T,
    tellp: <T>(T) -> number,
    clear: <T>(T) -> T,
}
type writer_meta = {
    __call: <T, U...>(T, U...) -> (),
}
--export type writer = typeof(setmetatable({} :: writer_impl, {} :: callable)) & (<T...>(T...) -> ())
type fd<T> = {
    is_open: (T) -> boolean,
    close: (T) -> (),
    close_after: <T>(T, (e: T) -> ()) -> (),
}
type printer = <T...>(T...) -> ()
export type file_writer = writer & fd<writer>
export type file_reader = reader & fd<reader>
type filesystem = {
    rename: (from: anypath, to: anypath) -> (),
    remove: (path: anypath) -> boolean,
    remove_all: (path: anypath) -> number,
    exists: (path: anypath) -> boolean,
    equivalent: (a: anypath, b: anypath) -> boolean,
    create_directory: (path: anypath) -> boolean,
    directory_iterator: (path: anypath, recursive: boolean?) -> iterator<path>,
    current_path: () -> path,
    is_directory: (path: anypath) -> boolean,
    is_regular_file: (path: string) -> boolean,
    temp_directory_path: () -> path,
    weakly_canonical: (path: anypath) -> path,
    canonical: (path: anypath) -> path,
    absolute: (path: anypath) -> path,
    copy: (from: anypath, to: anypath, opts: copy_options?) -> (),
    copy_file: (from: anypath, to: anypath, opts: copy_options?) -> boolean,
    copy_symlink: (from: anypath, to: anypath) -> (),
    create_symlink: (to: anypath, new_symlink: anypath) -> (),
    create_directory_symlink: (to: anypath, new_symlink: anypath) -> (),
    create_directories: (path: anypath) -> boolean,
    path: (path: string) -> path,
    find_in_environment: (environment_variable: string) -> path?,
    read_symlink: (symlink: anypath) -> path,
    home_path: () -> path,
}
type process = {
    system: (command: string) -> number,
    arg_iterator: () -> (() -> string?),
    args: () -> {string},
    sleep_for: (seconds: number) -> (), 
}
type garbage = {
    count: () -> number,
    collect: () -> (),
}
type io = {
    stdin: reader,
    stdout: writer,
    stderr: writer,
    scan: () -> string,
    errprint: <Ts...>(Ts...) -> (), 
    file_reader: (file: anypath) -> file_reader,
    file_writer: (file: anypath, append_mode: boolean?) -> file_writer,
}
type callable = {
    __call: (self: any, ...never) -> (...never),
}
type type = typeof(setmetatable({}, {} :: callable))
type plus = {
    proc: process,
    fs: filesystem,
    gc: garbage,
    io: io,
    scope: (<T>(T, block: (T) -> ()) -> ()) & (<T, U>(T, block: (T) -> U) -> ()),
    defer: <T>(T, deferred: (T) -> (), but_first: (T) -> ()) -> (),
    scan: () -> string,
    warn: <T...>(T...) -> (),
    print: <T...>(T...) -> (),
}
declare pls: plus
