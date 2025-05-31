cmake_minimum_required(VERSION 3.12)

project(CommandLineMenu-Example LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)

add_executable(example1 example1.cpp)