export type copy_options_t = "recursive" | "update existing" | "skip existing" | "create symlinks" | "copy symlinks" | "overwrite existing" | "directories only" | "create hard links" | "none"
type filesystem = {
    rename: (from: string, to: string) -> (),
    remove: (path: string) -> boolean,
    remove_all: (path: string) -> number,
    exists: (path: string) -> boolean,
    create_directory: (path: string) -> boolean,
    iterator: (path: string, recursive: boolean?) -> (() -> string?),
    current_path: () -> string,
    is_directory: (path: string) -> boolean,
    is_regular_file: (path: string) -> boolean,
    temp_directory_path: () -> string,
    weakly_canonical: (path: string) -> string,
    absolute: (path: string) -> string,
    copy: (from: string, to: string, opts: copy_options_t?) -> (),
    copy_file: (from: string, to: string, opts: copy_options_t?) -> boolean,
    copy_symlink: (from: string, to: string) -> (),
    create_symlink: (to: string, new_symlink: string) -> (),
    create_directory_symlink: (to: string, new_symlink: string) -> (),
    create_directories: (path: string) -> boolean,
}
declare fs: filesystem
declare collectgarbage: (("collect") -> ()) & (("count") -> number)
