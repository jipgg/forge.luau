add_executable(executable
    cli.cpp
    state.cpp
    lib_impl/require.cpp
    lib_impl/filesystem.cpp
    lib_impl/json.cpp
    lib_impl/fileio.cpp
    lib_impl/consoleio.cpp
    lib_impl/process.cpp
    lib_impl/garbage.cpp
    type_impl/filewriter.cpp
    type_impl/writer.cpp
    type_impl/path.cpp
    ../extern/luau/CLI/src/ReplRequirer.cpp
    ../extern/luau/CLI/src/RequirerUtils.cpp
    ../extern/luau/CLI/src/Coverage.cpp
)

target_link_libraries(executable PRIVATE
    Luau.VM
    Luau.Common
    Luau.Compiler
    Luau.Config
    Luau.CodeGen
    Luau.CLI.lib
    isocline
    Luau.Require
    Luau.RequireNavigator
    httplib
    nlohmann_json::nlohmann_json
)
target_include_directories(executable PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(executable PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
