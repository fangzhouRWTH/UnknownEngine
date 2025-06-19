cmake_minimum_required(VERSION 3.25.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
project(EXPERIMENT VERSION 1.0.0 LANGUAGES C CXX)

find_package(Eigen3 REQUIRED)
find_package(Vulkan REQUIRED)
#find_package(vk-bootstrap REQUIRED)
find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)
find_package(glfw3 REQUIRED)

add_subdirectory(expShader)

set(EXPERIMENT_SOURCE_FILES
    ${PROJECT_SOURCE_DIR}/vulkan_renderer/vma.cpp
    ${PROJECT_SOURCE_DIR}/vulkan_renderer/device.cpp
    ${PROJECT_SOURCE_DIR}/vulkan_renderer/swapchain.cpp
    ${PROJECT_SOURCE_DIR}/vulkan_renderer/deviceResourceAllocator.cpp
    ${PROJECT_SOURCE_DIR}/vulkan_renderer/resource.cpp
    ${PROJECT_SOURCE_DIR}/vulkan_renderer/resourceAllocator.cpp
    ${PROJECT_SOURCE_DIR}/vulkan_renderer/memAllocation.cpp
    #${PROJECT_SOURCE_DIR}/vulkan_renderer/image.cpp
    ${PROJECT_SOURCE_DIR}/vulkan_renderer/descriptor.cpp
    ${PROJECT_SOURCE_DIR}/vulkan_renderer/pipeline.cpp
    ${PROJECT_SOURCE_DIR}/vulkan_renderer/renderGraph.cpp
    ${PROJECT_SOURCE_DIR}/vulkan_renderer/shader.cpp
    ${PROJECT_SOURCE_DIR}/vulkan_renderer/command.cpp
    ${PROJECT_SOURCE_DIR}/vulkan_renderer/sync.cpp
    ${PROJECT_SOURCE_DIR}/vulkan_renderer/frame.cpp
    ${PROJECT_SOURCE_DIR}/vulkan_renderer/vkcore.cpp
    ${PROJECT_SOURCE_DIR}/vulkan_renderer/renderObject.cpp
    ${PROJECT_SOURCE_DIR}/exp.cpp
    #${PROJECT_SOURCE_DIR}/simpleVK.cpp
    ${PROJECT_SOURCE_DIR}/misc.cpp
    ${CMAKE_SOURCE_DIR}/UnknownEngine/source/project/core/math.cpp
    ${CMAKE_SOURCE_DIR}/UnknownEngine/source/project/core/input.cpp
    ${CMAKE_SOURCE_DIR}/UnknownEngine/source/project/debug/log.cpp
    ${CMAKE_SOURCE_DIR}/UnknownEngine/source/project/framework/framework.cpp
    ${CMAKE_SOURCE_DIR}/UnknownEngine/source/project/memory/allocator.cpp
    ${CMAKE_SOURCE_DIR}/UnknownEngine/source/project/ecs/ecs.cpp
    ${CMAKE_SOURCE_DIR}/UnknownEngine/source/project/core/clock.cpp
)
 
add_executable(EXPERIMENT ${EXPERIMENT_SOURCE_FILES})

add_dependencies(EXPERIMENT ExpShaders)

target_link_libraries(EXPERIMENT PRIVATE Eigen3::Eigen)
target_link_libraries(EXPERIMENT PRIVATE Vulkan::Vulkan)
target_link_libraries(EXPERIMENT PRIVATE spdlog::spdlog)
target_link_libraries(EXPERIMENT PRIVATE fmt::fmt)
target_link_libraries(EXPERIMENT PRIVATE vk-bootstrap::vk-bootstrap)
target_link_libraries(EXPERIMENT PRIVATE volk)
target_link_libraries(EXPERIMENT PRIVATE glfw)
target_link_libraries(UnknownEngine PRIVATE vulkan-headers::vulkan-headers)

target_include_directories(EXPERIMENT PUBLIC
                          "${CMAKE_SOURCE_DIR}/UnknownEngine/experiment"
                          "${CMAKE_SOURCE_DIR}/UnknownEngine/source/project"
                          "${CMAKE_SOURCE_DIR}/UnknownEngine/external/VKBOOTSTRAP/vk-bootstrap/src"
                          "${CMAKE_SOURCE_DIR}/UnknownEngine/external/VOLK/volk"
                          )