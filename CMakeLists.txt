cmake_minimum_required(VERSION 3.19)
project(cs8)

set(CMAKE_CXX_STANDARD 20)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake/Modules/cmake-pandocology")

add_subdirectory(cs8_emulator)
add_subdirectory(cs8_assembler)
add_subdirectory(cs8_doc)