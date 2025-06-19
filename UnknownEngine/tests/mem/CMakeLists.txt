cmake_minimum_required(VERSION 3.25.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
project(TEST_MEMORY VERSION 1.0.0 LANGUAGES C CXX)

find_package(Catch2 REQUIRED)

set(MEM_SOURCE_FILES
    "${CMAKE_SOURCE_DIR}/UnknownEngine/experiment/vulkan_renderer/memAllocation.cpp"
    "${PROJECT_SOURCE_DIR}/mem.cpp"
)

add_executable(TEST_MEMORY ${MEM_SOURCE_FILES})

target_link_libraries(TEST_MEMORY PRIVATE Catch2::Catch2WithMain)

target_include_directories(TEST_MEMORY PUBLIC
                          "${PROJECT_BINARY_DIR}"
                          )
                          
target_include_directories(TEST_MEMORY PUBLIC
                          "${CMAKE_SOURCE_DIR}/UnknownEngine/source/project"
                          "${CMAKE_SOURCE_DIR}/UnknownEngine/experiment"
                          )