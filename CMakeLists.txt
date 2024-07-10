cmake_minimum_required (VERSION 3.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project (PopoSDR CXX)

# lib srcs
file(GLOB_RECURSE Lib_srcs src/*.cpp include/*.h)

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/modems/include
)

# Add lib
add_library(LibPopoSDR STATIC ${Lib_srcs})

# Link lib with socket stuff for win
IF (WIN32)
    target_link_libraries(LibPopoSDR ws2_32 wsock32)
ENDIF()

# Modem srcs
file(GLOB_RECURSE Modem_srcs modems/*.cpp modems/include/*.h)

# Examples & Modems
add_executable(GMSK_Test examples/GMSK_Test.cpp ${Modem_srcs})
add_executable(AFSK_Test examples/AFSK_Test.cpp ${Modem_srcs})
add_executable(Main_Test main.cpp)
add_executable(WFM_Test examples/WFM_Mono_UDP.cpp)
add_executable(GMSK_Rx examples/GMSK_Rx.cpp)
add_executable(QAM_Rx examples/QAM_Rx.cpp)
add_executable(BerTest examples/BerTest.cpp ${Modem_srcs})

# Link with lib
target_link_libraries(GMSK_Test LibPopoSDR)
target_link_libraries(AFSK_Test LibPopoSDR)
target_link_libraries(Main_Test LibPopoSDR)
target_link_libraries(WFM_Test  LibPopoSDR)
target_link_libraries(GMSK_Rx   LibPopoSDR)
target_link_libraries(QAM_Rx    LibPopoSDR)
target_link_libraries(BerTest   LibPopoSDR)