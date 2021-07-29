cmake_minimum_required(VERSION 3.19)
project(cs8_assembler)

set(CMAKE_CXX_STANDARD 20)

find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

bison_target(CS8Parser src/cs8_asm.y ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp
        COMPILE_FLAGS -Wcounterexamples
        DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/parser.h )
flex_target(CS8Scanner src/cs8_asm.l ${CMAKE_CURRENT_BINARY_DIR}/scanner.cpp
        DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/scanner.h)

ADD_FLEX_BISON_DEPENDENCY(CS8Scanner CS8Parser)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_library(CS8_AssemblerLibrary
        ${BISON_CS8Parser_OUTPUTS}
        ${FLEX_CS8Scanner_OUTPUTS}
        src/Ast.cpp src/Ast.h src/cs8_parser.h src/MacroExpander.cpp src/MacroExpander.h src/AsmTree.cpp src/AsmTree.h src/AsmTreeTransformer.cpp src/AsmTreeTransformer.h src/asm_tree_emitter.cxx src/asm_tree_emitter.hxx src/CS8_Assembler.hxx src/cs8_assembler.cxx)
target_include_directories(CS8_AssemblerLibrary PUBLIC SYSTEM dependencies/ELFIO/)
target_include_directories(CS8_AssemblerLibrary PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(CS8_AssemblerLibrary PUBLIC ${FLEX_LIBRARIES})

add_executable(CS8_Assembler src/main.cpp)
target_link_libraries(CS8_Assembler CS8_AssemblerLibrary)