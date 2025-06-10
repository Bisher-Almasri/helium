cmake_minimum_required(VERSION 3.20)

project(helium)

set(CMAKE_CXX_STANDARD 20)

add_executable(hlc src/main.cpp
        src/tokenizer.hpp)


