cmake_minimum_required(VERSION 3.16)
project(logic_opt VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable testing
enable_testing()

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/src)

# Include subdirectories for header files
include_directories(${PROJECT_SOURCE_DIR}/src/parser)
include_directories(${PROJECT_SOURCE_DIR}/src/evaluator)
include_directories(${PROJECT_SOURCE_DIR}/src/minimizer)

# =============================================================================
# Google Test (manual path since we built it)
# =============================================================================
set(GTest_DIR /tmp/googletest/build)
include_directories(/tmp/googletest/googletest/include)
include_directories(/tmp/googletest/googlemock/include)

# =============================================================================
# Main Library (header-only, but compiled for testing)
# =============================================================================
set(LIB_SOURCES
    src/evaluator/truth_table.cpp
    src/minimizer/qmc.cpp
    src/minimizer/petrick.cpp
    src/minimizer/output.cpp
)

# =============================================================================
# Main Executable
# =============================================================================
set(SOURCES
    ${LIB_SOURCES}
    src/main.cpp
)

add_executable(logic_opt ${SOURCES})
target_include_directories(logic_opt PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(logic_opt pthread)

# =============================================================================
# Google Test Setup
# =============================================================================
set(GTEST_LIBS /tmp/googletest/build/lib/libgtest.a /tmp/googletest/build/lib/libgtest_main.a)

# Add test sources
set(TEST_SOURCES
    ${LIB_SOURCES}
    tests/test_expression.cpp
    tests/test_lexer.cpp
    tests/test_parser.cpp
    tests/test_truth_table.cpp
    tests/test_qmc.cpp
    tests/test_petrick.cpp
    tests/test_output.cpp
    tests/main.cpp
)

add_executable(logic_opt_tests ${TEST_SOURCES})
target_include_directories(logic_opt_tests PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(logic_opt_tests ${GTEST_LIBS} pthread)

# Add coverage flags if using GCC/Clang
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(logic_opt PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Werror
    )

    target_compile_options(logic_opt_tests PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        --coverage
        -fprofile-arcs
        -ftest-coverage
    )

    target_link_libraries(logic_opt_tests
        -lgcov
    )
endif()

# Add test target
add_test(NAME logic_opt_tests COMMAND logic_opt_tests)

# configure_file(end)
configure_file(${CMAKE_SOURCE_DIR}/tests/run_tests.sh ${CMAKE_BINARY_DIR}/run_tests.sh COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/tests/generate_coverage.sh ${CMAKE_BINARY_DIR}/generate_coverage.sh COPYONLY)

# =============================================================================
# Custom targets for convenience
# =============================================================================
# Using run_tests as the name instead of 'test' since 'test' is reserved by CTest
add_custom_target(run_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS logic_opt_tests
)

add_custom_target(coverage
    COMMAND ${CMAKE_BINARY_DIR}/generate_coverage.sh
    DEPENDS logic_opt_tests
)
