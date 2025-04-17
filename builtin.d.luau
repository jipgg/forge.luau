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
}
export type __builtin_json = {
    parse: <Ty>(json: string) -> Ty,
    to_json: <Ty>(json: Ty) -> string,
}
export type __builtin_process = {
    system: (command: string) -> number,
    args: {string},
}
export type __builtin_fileio = {
    write: (file_path: path_t, contents: string) -> (),
    read_all: (file_path: path_t) -> string,
    open_writer: (file: path_t, append_mode: boolean?) -> filewriter,
    append_line: (file: path_t, line: string) -> (),
    append: (file: path_t, txt: string) -> (),
}
export type __builtin_io = {
    stdin: () -> string,
    stderr: (txt: string) -> (),
    stdout: (txt: string) -> (),
}
export type __builtin = {
    process: __builtin_process,
    filesystem: __builtin_filesystem,
    json: __builtin_json,
    io: __builtin_io,
    fileio: __builtin_fileio,
}

export type __filewriter_scoped_ref = typeof(setmetatable(
    {} :: _Filewriter_scoped_ref,
    {} :: _Filewriter_scoped_ref_metatable
))
type _Filewriter_scoped_ref_metatable = {
    __concat: <T>(self: __filewriter_scoped_ref, other: T) -> __filewriter_scoped_ref,
}
type _Filewriter_scoped_ref = {
    write: (self: __filewriter_scoped_ref, str: string) -> (),
    is_open: (self: __filewriter_scoped_ref) -> boolean,
    line_iterator: (self: __filewriter_scoped_ref) -> (() -> string?),
    eof: (self: __filewriter_scoped_ref) -> boolean,
}
export type __filewriter_scope_fn = (self: __filewriter_scoped_ref) -> ()
declare class filewriter
    function write(self, str: string): ()
    function is_open(self): boolean
    function close(self): ()
    function line_iterator(self): (() -> string?)
    function scope(self, fn: __filewriter_scope_fn): (boolean?, string)
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
