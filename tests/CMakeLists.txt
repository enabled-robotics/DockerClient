cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

add_executable(test_docker test.cpp)
target_link_libraries(test_docker ${PROJECT_NAME})
