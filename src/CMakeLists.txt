cmake_minimum_required(VERSION 3.24.2 FATAL_ERROR)

add_library(
        ${PROJECT_NAME} docker.cpp
)

target_compile_definitions(${PROJECT_NAME} PRIVATE RAPIDJSON_HAS_STDSTRING)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/vendor/rapidjson/include/)

target_link_libraries(${PROJECT_NAME} CURL)
