cmake_minimum_required (VERSION 3.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project (PopoSDR CXX)

file(GLOB_RECURSE srcs src/*.cpp include/*.h modems/*.cpp modems/include/*.h)

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/modems/include
)

add_executable(${PROJECT_NAME} main.cpp ${srcs})