add_executable(lauf_fw WIN32
    main.cpp
    state_setup.cpp
    filesystem_library.cpp
    json_library.cpp
    fileio_library.cpp
    path_type.cpp
)

target_link_libraries(lauf_fw PRIVATE
    Luau.VM
    Luau.Common
    Luau.Compiler
    Luau.Config
    Luau.CodeGen
    Luau.CLI.lib
    httplib
    nlohmann_json::nlohmann_json
)
set_target_properties(lauf_fw PROPERTIES OUTPUT_NAME lauf)
