cmake_minimum_required(VERSION 3.0)
project(yslang)

set(CMAKE_CXX_STANDARD 14)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(CMAKE_CXX_FLAGS_DEBUG -g)
set(CMAKE_EXPORT_COMPILE_COMMANDS true)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

llvm_map_components_to_libnames(llvm_libs all)

add_subdirectory(src)
add_executable(ys src/main.cpp)
target_link_libraries(ys yslang ${llvm_libs})
# add_executable(llvmpl0 llvm_frontend.cpp lexer.cpp)
# target_link_libraries(llvmpl0 ${llvm_libs})

# add_custom_target(pl0lib DEPENDS write.ll)
# add_custom_command(OUTPUT write.ll
#   COMMAND clang -emit-llvm -S -O -o write.ll ${CMAKE_SOURCE_DIR}/write.c
# )
# add_dependencies(llvmpl0 pl0lib)
