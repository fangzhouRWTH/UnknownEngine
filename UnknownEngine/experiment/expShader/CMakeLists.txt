cmake_minimum_required(VERSION 3.25.0)
                       
find_program(GLSL_VALIDATOR glslangValidator HINTS /usr/bin /usr/local/bin $ENV{VULKAN_SDK}/Bin/ $ENV{VULKAN_SDK}/Bin32/)
file(GLOB_RECURSE GLSL_SOURCE_FILES_EXP
    "${PROJECT_SOURCE_DIR}/expShader/*.frag"
    "${PROJECT_SOURCE_DIR}/expShader/*.vert"
    "${PROJECT_SOURCE_DIR}/expShader/*.comp"
    "${PROJECT_SOURCE_DIR}/expShader/*.task"
    "${PROJECT_SOURCE_DIR}/expShader/*.mesh"
    )

foreach(GLSL_EXP ${GLSL_SOURCE_FILES_EXP})
  message(STATUS "BUILDING SHADER")
  get_filename_component(FILE_NAME ${GLSL_EXP} NAME)
  set(SPIRV "${PROJECT_SOURCE_DIR}/expShader/${FILE_NAME}.spv")
  message(STATUS COMMAND\ ${GLSL_VALIDATOR}\ -V\ --target-env\ vulkan1.4\ ${GLSL_EXP}\ -o\ ${SPIRV})
  add_custom_command(
    OUTPUT ${SPIRV}
    COMMAND ${GLSL_VALIDATOR} -V --target-env vulkan1.4 ${GLSL_EXP} -o ${SPIRV}
    DEPENDS ${GLSL_EXP})
  list(APPEND SPIRV_BINARY_FILES_EXP ${SPIRV})
endforeach(GLSL_EXP)

add_custom_target(ExpShaders DEPENDS ${SPIRV_BINARY_FILES_EXP})
#===================================shaders==================================
#add_dependencies(shadercompile Shaders)