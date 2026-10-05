cmake_minimum_required(VERSION 3.10)
project(Bookstore CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 收集 src 下所有 .cpp（新增文件无需改这里）
file(GLOB_RECURSE SOURCES ${CMAKE_SOURCE_DIR}/src/*.cpp)
add_executable(code ${SOURCES})

# 按作业要求，可执行文件名必须为 code
set_target_properties(code PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})