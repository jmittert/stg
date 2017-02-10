cmake_minimum_required(VERSION 2.8)
project( STG )

set(CMAKE_BUILD_TYPE Debug)
enable_testing()

include_directories(include ${CMAKE_CURRENT_BINARY_DIR})

find_package(BISON)
find_package(FLEX)
BISON_TARGET(stg_parser src/main/grammar/stg.yy ${CMAKE_CURRENT_BINARY_DIR}/parser.cc)
FLEX_TARGET(stg_scanner src/main/grammar/stg.ll ${CMAKE_CURRENT_BINARY_DIR}/lexer.cc)
ADD_FLEX_BISON_DEPENDENCY(stg_scanner stg_parser)

file(GLOB test_SRC "test/*.cc")

MESSAGE(${BISON_stg_parser_OUTPUT_SOURCE})
add_executable( stg ${BISON_stg_parser_OUTPUTS} ${FLEX_stg_scanner_OUTPUTS} src/main/main.cc src/lib/eval.cc src/lib/state.cc src/main/grammar/parser_driver.cc)
add_executable( testExe ${test_SRC} ${BISON_stg_parser_OUTPUTS} ${FLEX_stg_scanner_OUTPUTS} src/lib/eval.cc src/lib/state.cc src/main/grammar/parser_driver.cc)

target_link_libraries( testExe UnitTest++)
add_test( UnitTests testExe )

set (CMAKE_CXX_FLAGS "-std=c++11")
