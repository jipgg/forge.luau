type Filesystem = {
    rename: (from: string, to: string) -> (),
    remove: (path: string) -> (),
    remove_all: (path: string) -> (),
    write_file: (path: string, contents: string) -> (),
    new_directory: (path: string) -> (),
    iterator: (path: string, recursive: boolean?) -> (() -> string?),
    current_path: () -> string,
}

declare Fs: Filesystem
declare collectgarbage: (("collect") -> ()) & (("count") -> number)
