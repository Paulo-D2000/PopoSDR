cmake_minimum_required (VERSION 3.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project (PopoSDR CXX)

file(GLOB_RECURSE Lib_srcs src/*.cpp include/*.h)

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/modems/include
)

# Build MODEMS
file(GLOB_RECURSE Modem_srcs modems/*.cpp modems/include/*.h)

add_executable(GMSK_Test examples/GMSK_Test.cpp ${Lib_srcs} ${Modem_srcs})
add_executable(AFSK_Test examples/AFSK_Test.cpp ${Lib_srcs} ${Modem_srcs})
add_executable(Main_Test main.cpp ${Lib_srcs})
add_executable(WFM_Test examples/WFM_Mono_UDP.cpp ${Lib_srcs})
add_executable(GMSK_Rx examples/GMSK_Rx.cpp ${Lib_srcs})
add_executable(QAM_Rx examples/QAM_Rx.cpp ${Lib_srcs})