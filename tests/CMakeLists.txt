cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

macro(add_tests PREFIX TESTS)
    foreach (TEST ${TESTS})
        add_executable("${PREFIX}_${TEST}" ${TEST}.cpp)
        add_test(NAME "${PREFIX}_${TEST}" COMMAND "${PREFIX}_${TEST}")
        target_include_directories("${PREFIX}_${TEST}" PUBLIC ${PROJECT_NAME})
        target_link_libraries("${PREFIX}_${TEST}"
                gtest_main
                ${PROJECT_NAME})
    endforeach ()
endmacro()

add_compile_options(-Wall -Wextra -pedantic)
add_subdirectory(common)
