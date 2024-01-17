cmake_minimum_required (VERSION 3.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project (ModemGMSK CXX)

file(GLOB_RECURSE srcs src/*.cpp include/*.h)

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
)

add_executable(ModemGMSK main.cpp ${srcs})

target_link_directories(ModemGMSK PRIVATE "C:/Program Files/PothosSDR/lib/")

target_include_directories(ModemGMSK PRIVATE "C:/Program Files/PothosSDR/include/")

target_link_libraries(ModemGMSK PRIVATE SoapySDR)