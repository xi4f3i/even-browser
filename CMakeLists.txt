cmake_minimum_required(VERSION 3.31)

project(even-browser)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(fmt CONFIG REQUIRED)
find_package(unofficial-skia CONFIG REQUIRED)

add_executable(even-browser src/main.cpp)

target_link_libraries(even-browser PRIVATE fmt::fmt unofficial::skia::skia)
