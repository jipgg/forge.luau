add_executable(luauxt
    main.cpp
    state_setup.cpp
    filesystem_library.cpp
    json_library.cpp
    fileio_library.cpp
    path_type.cpp
)

target_link_libraries(luauxt PRIVATE
    Luau.VM
    Luau.Common
    Luau.Compiler
    Luau.Config
    Luau.CodeGen
    Luau.CLI.lib
    httplib
    nlohmann_json::nlohmann_json
)
set_target_properties(luauxt PROPERTIES OUTPUT_NAME luauxt)
