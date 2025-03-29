cmake_minimum_required(VERSION 3.0)

set(PROJECT_NAME "exampleDemo")
project(${PROJECT_NAME} VERSION 0.1 LANGUAGES CXX)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE logsweeper Threads::Threads)
