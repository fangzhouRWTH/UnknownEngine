#pragma once
#include <string>

#define API_OPENGL

namespace unknown::config
{
#ifdef API_OPENGL
    extern const std::string shader_folder_path;
#endif

    extern const std::string texture_folder_path;
    extern const std::string model_folder_path;
    extern const std::string scene_folder_path;

    extern const std::string config_folder_path;
}