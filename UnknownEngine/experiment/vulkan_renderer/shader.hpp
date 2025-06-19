#pragma once

#include "vulkan_renderer/common.hpp"
#include "utils/hash.hpp"
#include <string_view>
#include <string>
#include <unordered_map>

namespace unknown::renderer::vulkan
{
    enum class ShaderType
    {
        Vertex,Fragment,Task,Mesh,Compute,

        EnumMax
    };

    struct ShaderDesc
    {
        std::string path;
        ShaderType type;

        u64 hash() const {
            u64 h = 0;
            hash_combine(h,path);
            hash_combine(h,(u32)type);
            return h;
        }

        bool operator == (const ShaderDesc & rhs) const
        {
            return path==rhs.path && type==rhs.type;
        }
    };

    struct ShaderModule
    {
        VkShaderModule data;
        ShaderType type;
    };
}

namespace std {
  template <> struct hash<unknown::renderer::vulkan::ShaderDesc> {
    std::size_t operator()(
        const unknown::renderer::vulkan::ShaderDesc &key) const {
      return std::size_t(key.hash());
    }
  };
}

namespace unknown::renderer::vulkan
{
    class ShaderManager
    {
        public:
            void Init(VkDevice device){mDevice = device;}
            bool GetShaderModule(const ShaderDesc &desc, ShaderModule & shaderModule);
            void Clear();
        private:
            bool Create(const ShaderDesc &desc, ShaderModule & shaderModule);
            VkDevice mDevice;
            std::unordered_map<ShaderDesc,ShaderModule> mModules;
    };
}