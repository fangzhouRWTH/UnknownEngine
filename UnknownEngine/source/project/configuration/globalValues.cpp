#include "globalValues.hpp"

//#include <string>
//#define API_OPENGL

//TODO replace absolute path
namespace unknown::config
{
//#ifdef API_OPENGL
    
//#endif
// TODO 
    #ifdef __linux__
    const std::string shader_folder_path = "/home/fzl/workspace/git_projects/RenderEngineV0/UnknownEngine/shaders/";
    const std::string texture_folder_path = "/home/fzl/workspace/git_projects/RenderEngineV0/UnknownEngine/assets/images/textures/";
    const std::string model_folder_path = "/home/fzl/workspace/git_projects/RenderEngineV0/UnknownEngine/assets/models/";
    const std::string scene_folder_path = "/home/fzl/workspace/git_projects/RenderEngineV0/UnknownEngine/scene/";

    const std::string config_folder_path = "/home/fzl/workspace/git_projects/RenderEngineV0/UnknownEngine/build/config/";
    const std::string log_folder_path = "/home/fzl/workspace/git_projects/RenderEngineV0/log/";
    #elif defined(_WIN64)
    const std::string shader_folder_path = "C:/Users/franz/Downloads/Documents/Git/UnknownEngine/UnknownEngine/UnknownEngine/shaders/";
    const std::string texture_folder_path = "C:/Users/franz/Downloads/Documents/Git/UnknownEngine/UnknownEngine/UnknownEngine/assets/images/textures/";
    const std::string model_folder_path = "C:/Users/franz/Downloads/Documents/Git/UnknownEngine/UnknownEngine/UnknownEngine/assets/models/";
    const std::string scene_folder_path = "C:/Users/franz/Downloads/Documents/Git/UnknownEngine/UnknownEngine/UnknownEngine/scene/";

    const std::string config_folder_path = "C:/Users/franz/Downloads/Documents/Git/UnknownEngine/UnknownEngine/UnknownEngine/build/config/";
    const std::string log_folder_path = "C:/Users/franz/Downloads/Documents/Git/UnknownEngine/log/";
    // const std::string shader_folder_path = "C:/Fangzhou/Git/UnknownEngine/UnknownEngine/UnknownEngine/shaders/";
    // const std::string texture_folder_path = "C:/Fangzhou/Git/UnknownEngine/UnknownEngine/UnknownEngine/assets/images/textures/";
    // const std::string model_folder_path = "C:/Fangzhou/Git/UnknownEngine/UnknownEngine/UnknownEngine/assets/models/";
    // const std::string scene_folder_path = "C:/Fangzhou/Git/UnknownEngine/UnknownEngine/UnknownEngine/scene/";

    // const std::string config_folder_path = "C:/Fangzhou/Git/UnknownEngine/UnknownEngine/UnknownEngine/build/config/";
    // const std::string log_folder_path = "C:/Fangzhou/Git/UnknownEngine/log/";
    #endif
} // namespace unknown::config
