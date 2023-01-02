cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
project(DockerClient)

set(CMAKE_CXX_STANDARD 20)

find_package(CURL REQUIRED)

add_subdirectory(src)
add_subdirectory(tests)
