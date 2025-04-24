add_executable(executable
    cli.cpp
    state.cpp
    lib/filesystem.cpp
    lib/json.cpp
    lib/fileio.cpp
    lib/consoleio.cpp
    lib/process.cpp
    type/filewriter.cpp
    type/path.cpp
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
target_include_directories(executable PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(executable PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
