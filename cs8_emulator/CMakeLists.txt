cmake_minimum_required(VERSION 3.19)
project(cs8_emulator)

set(CMAKE_CXX_STANDARD 20)

set(${PROJECT_NAME}_SOURCES src/cpu.cxx src/cpu.hxx src/bus.cxx src/bus.hxx src/main.cxx src/devices.cxx src/devices.hxx src/device.cxx src/device.hxx src/memory.cxx src/memory.hxx src/serial_port.cxx src/serial_port.hxx)

add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC SYSTEM dependencies/ELFIO/)

find_package(SDL2 REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_LIBRARIES})