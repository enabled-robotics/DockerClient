cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
project(DockerClient)

set(CMAKE_CXX_STANDARD 23)

include(FindCURL)

add_subdirectory(src)

if (DOCKER_CLIENT_BUILD_TESTING)
    enable_testing()
    add_subdirectory(vendor/googletest-1.15.0)
    add_subdirectory(tests)
endif ()
