cmake_minimum_required(VERSION 3.20)
project(helium)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "LLVM include dir: ${LLVM_INCLUDE_DIRS}")
message(STATUS "LLVM library dir: ${LLVM_LIBRARY_DIRS}")
message(STATUS "LLVM used config from: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LLVM_CXXFLAGS}")

# Define your sources
add_executable(hlc
        src/main.cpp
        src/tokenization.hpp
        src/parser.hpp
        src/generation.hpp
        src/arena.hpp
)


target_link_libraries(hlc PRIVATE /usr/lib/libLLVM-20.so)

target_link_libraries(hlc PRIVATE
        pthread
        z
        zstd
        tinfo
        m
        dl
        rt
)
