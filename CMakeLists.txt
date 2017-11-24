cmake_minimum_required(VERSION 3.4.3)
project(al)


FIND_PACKAGE(BISON REQUIRED)
bison_target(parser parser.y ${CMAKE_CURRENT_SOURCE_DIR}/parser.tab.cpp)


# LLVM
set(LLVM_DIR "./llvm-5.0.0.src/build/lib/cmake/llvm")

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})
add_compile_options(-std=c++11)


add_executable(al al.cpp ${BISON_parser_OUTPUTS} lex.cpp)
llvm_map_components_to_libnames(llvm_libs support core irreader)
target_link_libraries(al ${llvm_libs} re2)
