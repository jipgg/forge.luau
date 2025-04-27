type copy_options_t = "recursive" | "update existing" | "skip existing" | "create symlinks" | "copy symlinks" | "overwrite existing" | "directories only" | "create hard links" | "none"
type file_type_t = "directory" | "symlink" | "file"
type path_t = __path | string
type path = __path
type writer = __writer
type filewriter = __filewriter
type filesystem = {
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
    stdout: writer,
    stderr: writer,
    filewriter: (file: path_t, append_mode: boolean?) -> filewriter,
}
type plus = {
    process: process,
    filesystem: filesystem,
    garbage: garbage,
    io: io,
}

declare class __writer
    function print(self, str: string): writer
    function println(self, str: string): writer
    function write(self, buf: buffer): writer
    function writeln(self, buf: buffer): writer
    function ln(self): writer
    function endl(self): writer
    function flush(self): writer
    function eof(self): boolean
    function __concat(self, other: any): writer
end
declare class __filewriter extends __writer
    function print(self, str: string): filewriter
    function println(self, str: string): filewriter
    function write(self, buf: buffer): filewriter
    function writeln(self, buf: buffer): filewriter
    function ln(self): filewriter
    function endl(self): filewriter
    function flush(self): filewriter
    function is_open(self): boolean
    function close(self): ()
    function close_after(self, fn: (self: writer) -> ()): (boolean?, string)
    function __concat(self, other: any): filewriter
end
declare class __path
    function string(self): string
    function generic_string(self): string
    function extension(self): path
    function has_extension(self): boolean
    function replace_extension(self, replacement: string): path
    function exist(self): boolean
    function parent(self): path
    function child(self, name: path_t): path
    function is_absolute(self): boolean
    function is_relative(self): boolean
    function filename(self): path
    function has_filename(self): boolean
    function replace_filename(self, replacement: string): path
    function remove_filename(self): path
    function remove_extension(self): path
    function is_directory(self): boolean
    function is_file(self): boolean
    function is_symlink(self): boolean
    function each_descendant(self): (() -> path?)
    function each_child(self): (() -> path?)
    function descendants(self): {path}
    function children(self): {path}
    function __div(self, other: path_t): path
end
declare plus: plus
