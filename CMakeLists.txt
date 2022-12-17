cmake_minimum_required(VERSION 2.8.3)
project(DockerClient)

set(CMAKE_CXX_STANDARD 20)

SET(ROOT ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(include)
include_directories(rapidjson/include)

add_executable(test_docker src/test.cpp)

add_library(
        docker-cpp SHARED
        src/docker.cpp
)

target_link_libraries(docker-cpp curl)
target_link_libraries(test_docker docker-cpp)

