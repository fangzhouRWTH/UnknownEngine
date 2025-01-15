cmake_minimum_required(VERSION 3.25.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
project(EXPERIMENT VERSION 1.0.0 LANGUAGES C CXX)

#find_package(Eigen3 REQUIRED)

set(EXPERIMENT_SOURCE_FILES
    "${PROJECT_SOURCE_DIR}/exp.cpp"
)

add_executable(EXPERIMENT ${EXPERIMENT_SOURCE_FILES})

#target_link_libraries(TEST_SCENE PRIVATE Eigen3::Eigen)
                          
target_include_directories(EXPERIMENT PUBLIC
                          "${CMAKE_SOURCE_DIR}/UnknownEngine/source/project"
                          "${CMAKE_SOURCE_DIR}/UnknownEngine/experiment/include"
                          )