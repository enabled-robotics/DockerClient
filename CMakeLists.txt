cmake_minimum_required(VERSION 2.8.3)
project(DockerClient)

set(CMAKE_CXX_STANDARD 11)

SET(ROOT ${CMAKE_CURRENT_SOURCE_DIR})

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${ROOT}/bin/${CMAKE_BUILD_TYPE}" CACHE PATH "Runtime directory" FORCE)
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${ROOT}/lib/${CMAKE_BUILD_TYPE}" CACHE PATH "Library directory" FORCE)
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${ROOT}/lib/${CMAKE_BUILD_TYPE}" CACHE PATH "Archive directory" FORCE)

include_directories(include)
include_directories(rapiedjson/include)

add_executable(test_docker src/test.cpp)

add_library(
    docker-cpp SHARED
    src/docker.cpp
)

target_link_libraries(docker-cpp curl)
target_link_libraries(test_docker docker-cpp)

