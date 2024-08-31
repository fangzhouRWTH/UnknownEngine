#include "globalValues.hpp"

//#include <string>
#define API_OPENGL

namespace unknown::config
{
#ifdef API_OPENGL
    const std::string shader_folder_path = "/home/fzl/workspace/git_projects/RenderEngineV0/UnknownEngine/shaders/";
#endif

    const std::string texture_folder_path = "/home/fzl/workspace/git_projects/RenderEngineV0/UnknownEngine/assets/images/textures/";
    const std::string model_folder_path = "/home/fzl/workspace/git_projects/RenderEngineV0/UnknownEngine/assets/models/";
    const std::string scene_folder_path = "/home/fzl/workspace/git_projects/RenderEngineV0/UnknownEngine/scene/";

    const std::string config_folder_path = "/home/fzl/workspace/git_projects/RenderEngineV0/UnknownEngine/build/config/";
} // namespace unknown::config
