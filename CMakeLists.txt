cmake_minimum_required(VERSION 3.15)
project(wow LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# language
add_subdirectory(extern/luau)
add_subdirectory(extern/httplib)
add_subdirectory(extern/nlohmannjson)

# source code
add_subdirectory(src/executable)
