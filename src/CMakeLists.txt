add_executable(executable
    cli.cpp
    state_setup.cpp
    filesystem.cpp
    json.cpp
    fileio.cpp
    filewriter.cpp
    path.cpp
)

target_link_libraries(executable PRIVATE
    Luau.VM
    Luau.Common
    Luau.Compiler
    Luau.Config
    Luau.CodeGen
    Luau.CLI.lib
    httplib
    nlohmann_json::nlohmann_json
)
set_target_properties(executable PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
