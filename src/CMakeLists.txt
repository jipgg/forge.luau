add_executable(executable
    cli.cpp
    state.cpp
    require.cpp
    lib/fs/library.cpp
    lib/io/library.cpp
    lib/http/library.cpp
    lib/json/library.cpp
    lib/proc/library.cpp
    lib/io/types.cpp
    lib/fs/path.cpp
    lib/http/client.cpp
    lib/http/response.cpp
    ${CMAKE_SOURCE_DIR}/extern/luau/CLI/src/ReplRequirer.cpp
    ${CMAKE_SOURCE_DIR}/extern/luau/CLI/src/RequirerUtils.cpp
    ${CMAKE_SOURCE_DIR}/extern/luau/CLI/src/Coverage.cpp
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
target_include_directories(executable PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
set_target_properties(executable PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
