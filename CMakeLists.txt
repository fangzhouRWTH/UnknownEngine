cmake_minimum_required(VERSION 3.25)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(UE)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#find package
find_package(Vulkan REQUIRED)
find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)
find_package(glfw3 REQUIRED)
find_package(assimp REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(stb REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(VulkanHeaders REQUIRED)
find_package(Catch2 REQUIRED)

add_subdirectory(UnknownEngine)