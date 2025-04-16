export type copy_options_t = "recursive" | "update existing" | "skip existing" | "create symlinks" | "copy symlinks" | "overwrite existing" | "directories only" | "create hard links" | "none"
export type path_t = path | string
type builtin_filesystem = {
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
}
type builtin_json = {
    parse: <Ty>(json: string) -> Ty,
    to_json: <Ty>(json: Ty) -> string,
}
type builtin_process = {
    system: (command: string) -> number,
    args: {string},
}
type builtin_fileio = {
    write_all: (file_path: path_t, contents: string) -> (),
    read_all: (file_path: path_t) -> string,
    open_writer: (file: path_t) -> file_writer,
    open_append_writer: (file: path_t) -> file_writer,
}
type builtin_io = {
    stdin: () -> string,
    stderr: (txt: string) -> (),
    stdout: (txt: string) -> (),
}
declare class file_writer
    function write(self, str: string): ()
    function close(self): ()
    function __add(self, other: any): file_writer
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
declare builtin: {
    process: builtin_process,
    filesystem: builtin_filesystem,
    json: builtin_json,
    io: builtin_io,
    fileio: builtin_fileio,
}

declare collectgarbage: (("collect") -> ()) & (("count") -> number)
