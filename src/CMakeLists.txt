set(LIBNAME "logsweeper")
add_library(${LIBNAME} STATIC logsweeper.cpp)
target_link_directories(${LIBNAME} PRIVATE Threads::Threads)
target_include_directories(${LIBNAME}  PUBLIC ${CMAKE_SOURCE_DIR})

