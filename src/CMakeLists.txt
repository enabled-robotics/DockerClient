cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

add_library(
        ${PROJECT_NAME} 
        docker.cpp
        curl.cpp
        docker_request_creator.cpp
        docker_response_processor.cpp
        query_creator.cpp
)

target_compile_definitions(${PROJECT_NAME} PRIVATE RAPIDJSON_HAS_STDSTRING)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/vendor/rapidjson/include/)

target_link_libraries(${PROJECT_NAME} ${CURL_LIBRARIES})
