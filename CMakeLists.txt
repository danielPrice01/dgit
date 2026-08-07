cmake_minimum_required(VERSION 3.20)

project(dgit VERSION 0.1 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

# Shared implementation used by both the main executable and tests.
file(GLOB_RECURSE DGIT_LIBRARY_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/utils/*.c"
)

# cli.c contains main(), so it should not be part of the shared library.
list(REMOVE_ITEM DGIT_LIBRARY_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/cli.c"
)

add_library(dgit_core STATIC
    ${DGIT_LIBRARY_SOURCES}
)

target_include_directories(dgit_core PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/utils"
    "${CMAKE_CURRENT_SOURCE_DIR}/utils/SHA1"
)

target_compile_definitions(dgit_core PUBLIC
    _DEFAULT_SOURCE
)

target_compile_options(dgit_core PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

add_executable(dgit
    src/cli.c
)

target_link_libraries(dgit PRIVATE
    dgit_core
)

file(GLOB TEST_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/testing/*.c"
)

add_executable(test_runner
    ${TEST_SOURCES}
)

target_link_libraries(test_runner PRIVATE
    dgit_core
)

enable_testing()

add_test(
    NAME dgit_tests
    COMMAND test_runner
)
