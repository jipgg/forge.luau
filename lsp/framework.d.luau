export type copy_options_t = "recursive" | "update existing" | "skip existing" | "create symlinks" | "copy symlinks" | "overwrite existing" | "directories only" | "create hard links" | "none"
export type fs_path_t = fs_path | string
type filesystem = {
    rename: (from: fs_path_t, to: fs_path_t) -> (),
    remove: (path: fs_path_t) -> boolean,
    remove_all: (path: fs_path_t) -> number,
    exists: (path: fs_path_t) -> boolean,
    create_directory: (path: fs_path_t) -> boolean,
    iterator: (path: fs_path_t, recursive: boolean?) -> (() -> fs_path?),
    current_path: () -> fs_path,
    is_directory: (path: fs_path_t) -> boolean,
    is_regular_file: (path: string) -> boolean,
    temp_directory_path: () -> fs_path,
    weakly_canonical: (path: fs_path_t) -> fs_path,
    absolute: (path: fs_path_t) -> fs_path,
    copy: (from: fs_path_t, to: fs_path_t, opts: copy_options_t?) -> (),
    copy_file: (from: fs_path_t, to: fs_path_t, opts: copy_options_t?) -> boolean,
    copy_symlink: (from: string, to: string) -> (),
    create_symlink: (to: fs_path_t, new_symlink: fs_path_t) -> (),
    create_directory_symlink: (to: fs_path_t, new_symlink: fs_path_t) -> (),
    create_directories: (path: fs_path_t) -> boolean,
    path: (path: string) -> fs_path,
}
declare class fs_path
    function string(self): string
    function extension(self): string
    function has_extension(self): boolean
    function replace_extension(self, replacement: string): fs_path
    function parent_path(self): fs_path
    function is_absolute(self): boolean
    function is_relative(self): boolean
    function filename(self): string
    function has_filename(self): boolean
    function replace_filename(self, replacement: string): fs_path
    function __div(self, other: fs_path | string): fs_path
end
declare fs: filesystem
declare collectgarbage: (("collect") -> ()) & (("count") -> number)
