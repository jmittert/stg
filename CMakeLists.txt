cmake_minimum_required(VERSION 2.8)
project( STG )

set(CMAKE_BUILD_TYPE Debug)
enable_testing()

find_package(bison required)
find_package(flex required)
BISON_TARGET(stg_parser src/main/grammar/stg.yy src/main/grammar/stg.tab.cc)
BISON_TARGET(stg_scanner src/main/grammar/stg.l src/main/grammar/lex.yy.cc)
ADD_FLEX_BISON_DEPENDENCY(stg_scanner stg_parser)

file(GLOB test_SRC "test/*.cpp")

add_executable( stg ${BISON_stg_parser_outputs} ${FLEX_stg_scanner_OUTPUTS} src/main/main.cc)
add_executable( testExe ${test_SRC} src/main/main.cc)

target_link_libraries( testExe UnitTest++)
add_test( testAll ./testExe )

set (CMAKE_CXX_FLAGS "-std=c++11")
