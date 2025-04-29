type copy_options = "recursive" | "update existing" | "skip existing" | "create symlinks" | "copy symlinks" | "overwrite existing" | "directories only" | "create hard links" | "none"
type file_type = "directory" | "symlink" | "file"
type anypath = path | string
export type result<V, E> = {
    and_then: <T, U>(T, (V) -> U) -> U,
    unpack: <T>(T) -> (V?, E?),
    value: <T>(T) -> V,
    error: <T>(T) -> E,
    has_value: <T>(T) -> boolean,
    has_error: <T>(T) -> boolean,
    value_or: <T>(T, v: V) -> V, 
    or_else: <T, U>(T, (E) -> U) -> U,
    error_or: <T>(T, e: E) -> E, 
    match: <T, U>(T, (V) -> U, (E) -> U) -> U,
    switch: <T>(T, (V) -> (), (E) -> ()) -> (),
}
export type path = {
    string: <T>(T) -> string,
    generic_string: <T>(T) -> string,
    extension: <T>(T) -> T,
    has_extension: <T>(T) -> boolean,
    replace_extension: <T>(T) -> T,
    parent: <T>(T) -> T,
    child: <T>(T, name: T | string) -> T,
    is_absolute: <T>(T) -> boolean,
    is_relative: <T>(T) -> boolean,
    filename: <T>(T) -> T,
    has_filename: <T>(T) -> boolean,
    replace_filename: <T>(T, filename: T | string) -> T,
    remove_filename: <T>(T) -> T,
    remove_extension: <T>(T) -> T,
    expect_directory: <T>(T) -> result<T, T>,
    is_directory: <T>(T) -> boolean,
    is_file: <T>(T) -> boolean,
    expect_file: <T>(T) -> result<T, T>,
    is_symlink: <T>(T) -> boolean,
    expect_symlink: <T>(T) -> result<T, T>,
    iterate_descendants: <T>(T) -> (() -> T?),
    iterate_children: <T>(T) -> (() -> T?),
    descendants: <T>(T) -> {T},
    children: <T>(T) -> {T},
    clone: <T>(T) -> T,
}
export type reader = {
    scan: <T>(T) -> string,
    getline: <T>(T, line: string) -> boolean,
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
    endl: <T>(T) -> T,
    flush: <T>(T) -> T,
    eof: <T>(T) -> boolean,
    good: <T>(T) -> boolean,
    bad: <T>(T) -> boolean,
    seekp: <T>(T, pos: number) -> T,
    tellp: <T>(T) -> number,
    clear: <T>(T) -> T,
    style: <T>(T, style: console_style) -> T
}
type file_descriptor = {
    is_open: <T>(T) -> boolean,
    close: <T>(T) -> (),
    close_after: <T>(T, (e: T) -> ()) -> (),
}
export type file_writer = file_descriptor & writer
export type file_reader = file_descriptor & reader
type filesystem = {
    rename: (from: anypath, to: anypath) -> (),
    remove: (path: anypath) -> boolean,
    remove_all: (path: anypath) -> number,
    exists: (path: anypath) -> boolean,
    equivalent: (a: anypath, b: anypath) -> boolean,
    create_directory: (path: anypath) -> boolean,
    directory_iterator: (path: anypath, recursive: boolean?) -> (() -> path?),
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
type __callable = {
    __call: (self: any, ...never) -> (...never),
}
type type = typeof(setmetatable({}, {} :: __callable))
type plus = {
    result: (<V, E>(val: V, nil) -> result<V, E>) & (<V, E>(nil, err: E) -> result<V, E>) & type,
    proc: process,
    fs: filesystem,
    garbage: garbage,
    io: io,
}
declare pls: plus
declare warn: <T...>(T...) -> ()
