add_executable(luf_fw WIN32
    main.cpp
    require.cpp
    filesystem.cpp
)

target_link_libraries(luf_fw PRIVATE
    Luau.VM
    Luau.Common
    Luau.Compiler
    Luau.Config
    Luau.CodeGen
    Luau.CLI.lib
)
set_target_properties(luf_fw PROPERTIES OUTPUT_NAME luf)
