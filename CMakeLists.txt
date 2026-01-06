cmake_minimum_required(VERSION 3.10)

project(even-browser)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(fmt CONFIG REQUIRED)
find_package(SDL3 CONFIG REQUIRED)
find_package(unofficial-skia CONFIG REQUIRED)

enable_testing()

set(EVEN_CORE_SOURCES
    src/html/tokenizer.cpp
)

set(EVEN_CORE_HEADERS
    src/dom/attr.h
    src/dom/document.h
    src/dom/element.h
    src/dom/node.h
    src/dom/text.h
    src/html/state.h
    src/html/token.h
    src/html/tokenizer.h
    src/util/char_util.h
)

add_library(even-core STATIC ${EVEN_CORE_SOURCES} ${EVEN_CORE_HEADERS})

target_link_libraries(even-core PUBLIC fmt::fmt SDL3::SDL3 unofficial::skia::skia)

target_include_directories(even-core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_executable(even-browser src/main.cpp)

target_link_libraries(even-browser PRIVATE even-core)

add_subdirectory(tests)