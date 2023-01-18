cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
project(DockerClient)

set(CMAKE_CXX_STANDARD 20)

include(FindCURL)

add_subdirectory(src)
add_subdirectory(tests)
