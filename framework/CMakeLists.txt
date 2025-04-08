add_executable(lauf_fw WIN32
    main.cpp
    state_setup.cpp
    fs_library.cpp
    fs_path.cpp
    http.cpp
)

target_link_libraries(lauf_fw PRIVATE
    Luau.VM
    Luau.Common
    Luau.Compiler
    Luau.Config
    Luau.CodeGen
    Luau.CLI.lib
    httplib
)
set_target_properties(lauf_fw PROPERTIES OUTPUT_NAME lauf)
