cmake_minimum_required(VERSION 3.0)

set(PROJECT_NAME "LogSweeper")
project(${PROJECT_NAME} VERSION 0.1 LANGUAGES CXX)

option(BUILD_EXAMPLES "build example" ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
message(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_subdirectory(src)

if (BUILD_EXAMPLES)
    add_subdirectory(example)
endif()
