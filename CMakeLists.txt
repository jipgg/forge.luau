cmake_minimum_required(VERSION 3.15)
project(lauf LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# language
add_subdirectory(luau)
add_subdirectory(extern/httplib)

# framework
add_subdirectory(framework)
