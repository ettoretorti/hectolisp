cmake_minimum_required (VERSION 3.1)
project (hectolisp)

option(BUILD_TESTS "Build the unit tests" OFF)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB HLISP_FILES "src/*.cpp" "src/*.c")

add_library(hlisp ${HLISP_FILES})

target_include_directories(
    hlisp
    PRIVATE ${CMAKE_SOURCE_DIR}/src)

target_compile_options(
    hlisp
    PRIVATE -Wall)

if(NOT "${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")
    #gcc options
    target_compile_options(
        hlisp
        PRIVATE -Wextra
        PRIVATE -pedantic)
    if(CMAKE_BUILD_TYPE EQUAL "DEBUG")
        target_compile_options(
            hlisp
	    PRIVATE -fsanitize=address)
    endif()
endif()


if(BUILD_TESTS)
    enable_testing()
    find_package(GTest REQUIRED)
    
    add_executable(alltests "test/all_tests.cpp")
    target_link_libraries(alltests hlisp ${GTEST_BOTH_LIBRARIES})
    target_include_directories(
        alltests
        PRIVATE ${CMAKE_SOURCE_DIR}/src
        PRIVATE ${CMAKE_SOURCE_DIR}/test)

    add_test(NAME alltests COMMAND alltests)

    add_executable(reader "src/reader/reader.c")
    target_link_libraries(reader hlisp)
    target_include_directories(
	reader
        PRIVATE ${CMAKE_SOURCE_DIR}/src)
    
    add_executable(evaluator "src/evaluator/evaluator.c")
    target_link_libraries(evaluator hlisp)
    target_include_directories(
	evaluator
        PRIVATE ${CMAKE_SOURCE_DIR}/src)
endif()

