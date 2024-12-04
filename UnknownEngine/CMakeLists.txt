cmake_minimum_required(VERSION 3.25.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
project(UnknownEngine VERSION 0.1.0 LANGUAGES C CXX)

find_package(Vulkan REQUIRED)
find_package(vk-bootstrap REQUIRED)
find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)
find_package(glfw3 REQUIRED)
find_package(assimp REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(stb REQUIRED)
find_package(nlohmann_json REQUIRED)

add_subdirectory(shaders)

set(RENDERER_SOURCE_FILES
    ${CMAKE_SOURCE_DIR}/UnknownEngine/source/project/app/main.cpp
    ${CMAKE_SOURCE_DIR}/UnknownEngine/source/project/app/engine.cpp
    ${CMAKE_SOURCE_DIR}/UnknownEngine/source/project/app/application.cpp
    ${CMAKE_SOURCE_DIR}/UnknownEngine/source/project/app/getApp.cpp
    ${CMAKE_SOURCE_DIR}/UnknownEngine/source/project/app/stringTable.cpp

    ${CMAKE_SOURCE_DIR}/UnknownEngine/source/project/renderer/renderer.cpp

    ${CMAKE_SOURCE_DIR}/UnknownEngine/source/project/renderer/vulkan/api_vulkan.cpp
    ${CMAKE_SOURCE_DIR}/UnknownEngine/source/project/renderer/vulkan/vkCore.cpp
    ${CMAKE_SOURCE_DIR}/UnknownEngine/source/project/renderer/vulkan/vkInitializer.cpp
    ${CMAKE_SOURCE_DIR}/UnknownEngine/source/project/renderer/vulkan/vkUtils.cpp
    ${CMAKE_SOURCE_DIR}/UnknownEngine/source/project/renderer/vulkan/vkDescriptor.cpp
    ${CMAKE_SOURCE_DIR}/UnknownEngine/source/project/renderer/vulkan/vkPipeline.cpp

    #${CMAKE_SOURCE_DIR}/source/project/renderer/shader.cpp

    ${CMAKE_SOURCE_DIR}/UnknownEngine/source/project/serialization/jsonSerializer.cpp

    ${CMAKE_SOURCE_DIR}/UnknownEngine/source/project/configuration/globalValues.cpp

    ${CMAKE_SOURCE_DIR}/UnknownEngine/source/project/debug/log.cpp

    ${CMAKE_SOURCE_DIR}/UnknownEngine/source/project/core/math.cpp
    ${CMAKE_SOURCE_DIR}/UnknownEngine/source/project/core/hash.cpp

    ${CMAKE_SOURCE_DIR}/UnknownEngine/source/project/core/input.cpp

    ${CMAKE_SOURCE_DIR}/UnknownEngine/source/project/core/clock.cpp

    ${CMAKE_SOURCE_DIR}/UnknownEngine/source/project/framework/framework.cpp

    ${CMAKE_SOURCE_DIR}/UnknownEngine/source/project/memory/allocator.cpp

    ${CMAKE_SOURCE_DIR}/UnknownEngine/source/project/ecs/ecs.cpp
    ${CMAKE_SOURCE_DIR}/UnknownEngine/source/project/renderer/renderSystem/renderSystem.cpp
    ${CMAKE_SOURCE_DIR}/UnknownEngine/source/project/renderer/rendererHandles.cpp

    ${CMAKE_SOURCE_DIR}/UnknownEngine/source/project/asset/modelLoader.cpp
    ${CMAKE_SOURCE_DIR}/UnknownEngine/source/project/asset/assetManager.cpp
    #${CMAKE_SOURCE_DIR}/source/project/asset/assetManager.cpp
    ${CMAKE_SOURCE_DIR}/UnknownEngine/source/project/asset/textureLoader.cpp
    ${CMAKE_SOURCE_DIR}/UnknownEngine/source/project/asset/utils/assimpHelper.cpp
    #{CMAKE_SOURCE_DIR}/source/project/asset/texture.cpp

    ${CMAKE_SOURCE_DIR}/UnknownEngine/source/project/world/world.cpp
    ${CMAKE_SOURCE_DIR}/UnknownEngine/source/project/world/scene.cpp

    ${CMAKE_SOURCE_DIR}/UnknownEngine/source/project/renderer/gui/simpleGUI.cpp

    #todo move out as dependency
    ${CMAKE_SOURCE_DIR}/UnknownEngine/source/external/IMGUI/vulkan_glfw/src/imgui.cpp
    ${CMAKE_SOURCE_DIR}/UnknownEngine/source/external/IMGUI/vulkan_glfw/src/imgui_draw.cpp
    ${CMAKE_SOURCE_DIR}/UnknownEngine/source/external/IMGUI/vulkan_glfw/src/imgui_tables.cpp
    ${CMAKE_SOURCE_DIR}/UnknownEngine/source/external/IMGUI/vulkan_glfw/src/imgui_widgets.cpp
    ${CMAKE_SOURCE_DIR}/UnknownEngine/source/external/IMGUI/vulkan_glfw/src/imgui_impl_vulkan.cpp
    ${CMAKE_SOURCE_DIR}/UnknownEngine/source/external/IMGUI/vulkan_glfw/src/imgui_impl_glfw.cpp
    )

    add_executable(UnknownEngine ${RENDERER_SOURCE_FILES})

    target_link_libraries(UnknownEngine PRIVATE Vulkan::Vulkan)
    target_link_libraries(UnknownEngine PRIVATE spdlog::spdlog)
    target_link_libraries(UnknownEngine PRIVATE fmt::fmt)
    target_link_libraries(UnknownEngine PRIVATE vk-bootstrap::vk-bootstrap)
    target_link_libraries(UnknownEngine PRIVATE glfw)
    target_link_libraries(UnknownEngine PRIVATE assimp::assimp)
    target_link_libraries(UnknownEngine PRIVATE Eigen3::Eigen)
    target_link_libraries(UnknownEngine PRIVATE stb::stb)
    target_link_libraries(UnknownEngine PRIVATE nlohmann_json::nlohmann_json)
    target_link_libraries(UnknownEngine PRIVATE vulkan-headers::vulkan-headers)

add_dependencies(UnknownEngine Shaders)

target_include_directories(UnknownEngine PUBLIC
                          "${PROJECT_BINARY_DIR}"
                          )

target_include_directories(UnknownEngine PRIVATE
                          "${PROJECT_SOURCE_DIR}/source/external/IMGUI/vulkan_glfw/include"
                          )
                          
target_include_directories(UnknownEngine PUBLIC
                          "${PROJECT_SOURCE_DIR}/source/project"
                          )


add_subdirectory(tests)
# set(CPACK_PROJECT_NAME ${PROJECT_NAME})
# set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
# include(CPack)
