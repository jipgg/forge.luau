export type copy_options_t = "recursive" | "update existing" | "skip existing" | "create symlinks" | "copy symlinks" | "overwrite existing" | "directories only" | "create hard links" | "none"
export type path_t = path | string
export type __builtin_filesystem = {
    rename: (from: path_t, to: path_t) -> (),
    remove: (path: path_t) -> boolean,
    remove_all: (path: path_t) -> number,
    exists: (path: path_t) -> boolean,
    equivalent: (a: path_t, b: path_t) -> boolean,
    create_directory: (path: path_t) -> boolean,
    directory_iterator: (path: path_t, recursive: boolean?) -> (() -> path?),
    current_path: () -> path,
    is_directory: (path: path_t) -> boolean,
    is_regular_file: (path: string) -> boolean,
    temp_directory_path: () -> path,
    weakly_canonical: (path: path_t) -> path,
    canonical: (path: path_t) -> path,
    absolute: (path: path_t) -> path,
    copy: (from: path_t, to: path_t, opts: copy_options_t?) -> (),
    copy_file: (from: path_t, to: path_t, opts: copy_options_t?) -> boolean,
    copy_symlink: (from: path_t, to: path_t) -> (),
    create_symlink: (to: path_t, new_symlink: path_t) -> (),
    create_directory_symlink: (to: path_t, new_symlink: path_t) -> (),
    create_directories: (path: path_t) -> boolean,
    path: (path: string) -> path,
    find_in_environment: (environment_variable: string) -> path?,
    read_symlink: (symlink: path_t) -> path,
}
export type __builtin_json = {
    parse: <Ty>(json: string) -> Ty,
    to_json: <Ty>(json: Ty) -> string,
}
export type __builtin_process = {
    system: (command: string) -> number,
    arg_iterator: () -> (() -> string?),
}
export type __builtin_fileio = {
    write_string: (file_path: path_t, contents: string) -> (),
    write: (file_path: path_t, contents: buffer) -> (),
    read_to_end: (file_path: path_t) -> buffer,
    open_writer: (file: path_t, append_mode: boolean?) -> filewriter,
    append_line: (file: path_t, line: string) -> (),
    append: (file: path_t, txt: string) -> (),
}
export type __builtin_consoleio = {
    scan: () -> string,
    read: (() -> buffer) & ((buf: buffer) -> ()) & ((count: number) -> buffer),
    err_write: (txt: string) -> (),
    write: (txt: string) -> (),
}
export type __builtin = {
    proc: __builtin_process,
    fs: __builtin_filesystem,
    json: __builtin_json,
    fio: __builtin_fileio,
    cio: __builtin_consoleio,
}

export type __filewriter_scoped_ref = typeof(setmetatable(
    {} :: _t_filewriter_scoped_ref,
    {} :: _mt_filewriter_scoped_ref
))
type _mt_filewriter_scoped_ref = {
    __concat: <T>(self: __filewriter_scoped_ref, other: T) -> __filewriter_scoped_ref,
}
type _t_filewriter_scoped_ref = {
    write_string: (self: __filewriter_scoped_ref, str: string) -> (),
    write: (self: __filewriter_scoped_ref, buf: buffer) -> (),
    is_open: (self: __filewriter_scoped_ref) -> boolean,
    eof: (self: __filewriter_scoped_ref) -> boolean,
}
export type __filewriter_scope_fn = (self: __filewriter_scoped_ref) -> ()
declare class filewriter
    function write_string(self, str: string): ()
    function write(self, buf: buffer): ()
    function is_open(self): boolean
    function close(self): ()
    function close_after(self, fn: __filewriter_scope_fn): (boolean?, string)
    function eof(self): boolean
    function __concat(self, other: any): filewriter
end
declare class path
    directory: path
    function string(self): string
    function generic_string(self): string
    function extension(self): string
    function has_extension(self): boolean
    function replace_extension(self, replacement: string): path
    function parent_path(self): path
    function is_absolute(self): boolean
    function is_relative(self): boolean
    function filename(self): string
    function has_filename(self): boolean
    function replace_filename(self, replacement: string): path
    function __div(self, other: path_t): path
end
declare builtin: __builtin

declare collectgarbage: (("collect") -> ()) & (("count") -> number)
