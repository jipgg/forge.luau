cmake_minimum_required(VERSION 3.15)
project(luauxl LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# language
add_subdirectory(luau)
add_subdirectory(extern/httplib)
add_subdirectory(extern/nlohmann_json)

# source code
add_subdirectory(src)
