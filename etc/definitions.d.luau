export type copy_options_t = "recursive" | "update existing" | "skip existing" | "create symlinks" | "copy symlinks" | "overwrite existing" | "directories only" | "create hard links" | "none"
export type file_type_t = "directory" | "symlink" | "file"
export type path_t = __path | string
export type path = __path
export type filewriter = __filewriter
export type __filesystem = {
    rename: (from: path_t, to: path_t) -> (),
    remove: (path: path_t) -> boolean,
    remove_all: (path: path_t) -> number,
    exists: (path: path_t) -> boolean,
    equivalent: (a: path_t, b: path_t) -> boolean,
    create_directory: (path: path_t) -> boolean,
    directory_iterator: (path: path_t, recursive: boolean?) -> (() -> __path?),
    current_path: () -> __path,
    is_directory: (path: path_t) -> boolean,
    is_regular_file: (path: string) -> boolean,
    temp_directory_path: () -> __path,
    weakly_canonical: (path: path_t) -> __path,
    canonical: (path: path_t) -> __path,
    absolute: (path: path_t) -> __path,
    copy: (from: path_t, to: path_t, opts: copy_options_t?) -> (),
    copy_file: (from: path_t, to: path_t, opts: copy_options_t?) -> boolean,
    copy_symlink: (from: path_t, to: path_t) -> (),
    create_symlink: (to: path_t, new_symlink: path_t) -> (),
    create_directory_symlink: (to: path_t, new_symlink: path_t) -> (),
    create_directories: (path: path_t) -> boolean,
    path: (path: string) -> __path,
    find_in_environment: (environment_variable: string) -> __path?,
    read_symlink: (symlink: path_t) -> __path,
    home_path: () -> __path,
}
export type __json = {
    parse: <Ty>(json: string) -> Ty,
    to_json: <Ty>(json: Ty) -> string,
}
export type __process = {
    system: (command: string) -> number,
    arg_iterator: () -> (() -> string?),
    args: () -> {string},
    sleep_for: (seconds: number) -> (), 
}
export type __garbage_collector = {
    count: () -> number,
    collect: () -> (),
}
export type __fileio = {
    write_string: (file_path: path_t, contents: string) -> (),
    write: (file_path: path_t, contents: buffer) -> (),
    read_to_end: (file_path: path_t) -> buffer,
    open_writer: (file: path_t, append_mode: boolean?) -> __filewriter,
    append_line: (file: path_t, line: string) -> (),
    append: (file: path_t, txt: string) -> (),
}
export type __consoleio = {
    scanln: () -> string,
    read: (() -> buffer) & ((buf: buffer) -> ()) & ((count: number) -> buffer),
    readln: (()->buffer) & ((buf: buffer) -> boolean),
    err_write: (buf: buffer) -> (),
    err_writeln: (buf: buffer) -> (),
    err_print: (txt: string) -> (),
    err_println: (txt: string) -> (),
    write: (data: buffer) -> (),
    writeln: (data: buffer) -> (),
    print: (txt: string) -> (),
    println: (txt: string) -> (),
}
export type __plus = {
    process: __process,
    filesystem: __filesystem,
    json: __json,
    fileio: __fileio,
    consoleio: __consoleio,
    garbage: __garbage_collector,
    stdout: plus_writer,
    stderr: plus_writer,
}

export type __filewriter_scoped_ref = typeof(setmetatable(
    {} :: _t_filewriter_scoped_ref,
    {} :: _mt_filewriter_scoped_ref
))
type _mt_filewriter_scoped_ref = {
    __concat: <T>(self: __filewriter_scoped_ref, other: T) -> __filewriter_scoped_ref,
}
type _t_filewriter_scoped_ref = {
    print: (self: __filewriter_scoped_ref, str: string) -> (),
    println: (self: __filewriter_scoped_ref, str: string) -> (),
    write: (self: __filewriter_scoped_ref, buf: buffer) -> (),
    writeln: (self: __filewriter_scoped_ref, buf: buffer) -> (),
    is_open: (self: __filewriter_scoped_ref) -> boolean,
    eof: (self: __filewriter_scoped_ref) -> boolean,
}
export type __filewriter_scope_fn = (self: __filewriter_scoped_ref) -> ()
declare class plus_writer
    function print(self, str: string): plus_writer
    function println(self, str: string): plus_writer
    function write(self, buf: buffer): plus_writer
    function writeln(self, buf: buffer): plus_writer
    function ln(self): plus_writer
    function endl(self): plus_writer
    function flush(self): plus_writer
    function eof(self): boolean
    function __concat(self, other: any): plus_writer
end
declare class __filewriter extends plus_writer
    function is_open(self): boolean
    function close(self): ()
    function close_after(self, fn: __filewriter_scope_fn): (boolean?, string)
    function __concat(self, other: any): __filewriter
end
declare class __path
    directory: __path
    function string(self): string
    function generic_string(self): string
    function extension(self): string
    function has_extension(self): boolean
    function replace_extension(self, replacement: string): __path
    function parent_path(self): __path
    function is_absolute(self): boolean
    function is_relative(self): boolean
    function filename(self): string
    function has_filename(self): boolean
    function replace_filename(self, replacement: string): __path
    function is_directory(self): boolean
    function is_file(self): boolean
    function is_symlink(self): boolean
    function directory_iterator(self, recursive: boolean?): (() -> __path?)
    function __div(self, other: path_t): __path
end
declare plus: __plus

declare collectgarbage: (("collect") -> ()) & (("count") -> number)
