cmake_minimum_required(VERSION 3.25.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
project(TEST_SCENE VERSION 1.0.0 LANGUAGES C CXX)

find_package(Catch2 REQUIRED)
find_package(Eigen3 REQUIRED)

set(SCENE_SOURCE_FILES
    "${PROJECT_SOURCE_DIR}/test_scene.cpp"
    "${CMAKE_SOURCE_DIR}/UnknownEngine/source/project/world/scene.cpp"
)

add_executable(TEST_SCENE ${SCENE_SOURCE_FILES})

target_link_libraries(TEST_SCENE PRIVATE Catch2::Catch2WithMain)
target_link_libraries(TEST_SCENE PRIVATE Eigen3::Eigen)

target_include_directories(TEST_SCENE PUBLIC
                          "${PROJECT_BINARY_DIR}"
                          )
                          
target_include_directories(TEST_SCENE PUBLIC
                          "${CMAKE_SOURCE_DIR}/UnknownEngine/source/project"
                          )