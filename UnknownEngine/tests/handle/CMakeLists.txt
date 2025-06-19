cmake_minimum_required(VERSION 3.25.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
project(TEST_HANDLE VERSION 1.0.0 LANGUAGES C CXX)

find_package(Catch2 REQUIRED)
#find_package(Eigen3 REQUIRED)

set(HANDLE_SOURCE_FILES
    "${PROJECT_SOURCE_DIR}/test_handle.cpp"
    #"${CMAKE_SOURCE_DIR}/UnknownEngine/source/project/world/scene.cpp"
)

add_executable(TEST_HANDLE ${HANDLE_SOURCE_FILES})

target_link_libraries(TEST_HANDLE PRIVATE Catch2::Catch2WithMain)
#target_link_libraries(TEST_SCENE PRIVATE Eigen3::Eigen)

target_include_directories(TEST_HANDLE PUBLIC
                          "${PROJECT_BINARY_DIR}"
                          )
                          
target_include_directories(TEST_HANDLE PUBLIC
                          "${CMAKE_SOURCE_DIR}/UnknownEngine/source/project"
                          "${CMAKE_SOURCE_DIR}/UnknownEngine/experiment"
                          )