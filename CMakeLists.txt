cmake_minimum_required(VERSION 3.10)
project(kprocview LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(kprocview
    src/main.cpp
    src/process.cpp
    src/system.cpp
    src/ui.cpp
)

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

target_link_libraries(kprocview PRIVATE ${CURSES_LIBRARIES})