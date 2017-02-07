cmake_minimum_required(VERSION 2.8)
project( DBDisplay )

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
enable_testing()

file(GLOB test_SRC "test/*.cpp")

add_executable( stg src/main/nnw.cpp src/lib/lib.cpp src/lib/features.cpp src/lib/cvlib.cpp)
add_executable( testExe ${test_SRC} src/lib/client.cpp src/lib/server.cpp src/lib/communicator.cpp src/lib/lib.cpp)

target_link_libraries( testExe UnitTest++)
add_test( testAll ./testExe )

set (CMAKE_CXX_FLAGS "-std=c++11")
