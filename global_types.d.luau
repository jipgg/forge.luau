export type copy_options_t = "recursive" | "update existing" | "skip existing" | "create symlinks" | "copy symlinks" | "overwrite existing" | "directories only" | "create hard links" | "none"
export type file_type_t = "directory" | "symlink" | "file"
export type path_t = __path | string
export type path = __path
export type fwriter = __fwriter
export type writer = __writer
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
    open_file: (path: path_t) -> __file,
    create_file: (path: path_t) -> __file,
    touch_file: (path: path_t) -> __file,
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
    exit: (exit_code: number?) -> never,
    bind_to_exit: (fn: ()->(), identifier: string?) -> (),
    unbind_from_exit: (identifier: string) -> boolean,
}
export type __io = {
    stderr: __writer,
    stdout: __writer,
    file: (path: path_t) -> __file,
}
export type __etc = {
    proc: __process,
    fs: __filesystem,
    json: __json,
    io: __io,
}
export type __fwriter_close_after = (self: writer) -> ()

declare class __writer
    function print(self, str: string): ()
    function println(self, str: string): ()
    function write(self, buf: buffer): ()
    function writeln(self, buf: buffer): ()
    function eof(self): boolean
end
declare class __fwriter extends __writer
    function is_open(self): boolean
    function close(self): ()
    function close_after(self, fn: __fwriter_close_after): (boolean?, string)
end
declare class __file extends __writer
    function writer(self): __writer
    function is_open(self): boolean
    function path(self): __path
    function close(self): ()
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
declare etc: __etc

declare collectgarbage: (("collect") -> ()) & (("count") -> number)
