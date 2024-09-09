cmake_minimum_required(VERSION 3.25.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
project(TEST_STRUCTURE VERSION 1.0.0 LANGUAGES C CXX)

find_package(Catch2 REQUIRED)

set(STRUCTURE_SOURCE_FILES
    "${PROJECT_SOURCE_DIR}/test_structure.cpp"
)

add_executable(TEST_STRUCTURE ${STRUCTURE_SOURCE_FILES})

target_link_libraries(TEST_STRUCTURE PRIVATE Catch2::Catch2WithMain)

target_include_directories(TEST_STRUCTURE PUBLIC
                          "${PROJECT_BINARY_DIR}"
                          )
                          
target_include_directories(TEST_STRUCTURE PUBLIC
                          "${CMAKE_SOURCE_DIR}/UnknownEngine/source/project"
                          )