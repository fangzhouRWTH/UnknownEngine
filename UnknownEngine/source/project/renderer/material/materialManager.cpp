#include "materialManager.hpp"
#include "renderer/renderer.hpp"

#include <concepts>

#include "renderer/vulkan/vkCore.hpp"

namespace unknown::renderer
{
    const std::string u_global_scene_data = "global_scene_data";

    template<typename M>
    MaterialClassInfo & RegisterMaterial(MaterialClassMap & materialInfoContainer)
    {
        static_assert(std::is_base_of_v<MaterialBase<M>,M>);
        MaterialClassID mID = M::GetMaterialClassID();
        MaterialClassInfo info;
        info.vertexShader = M::GetVertexShaderName();
        info.fragmentShader = M::GetFragmentShaderName();
        auto res = materialInfoContainer.insert({mID,info});
        assert(res.second);
        return res.first->second;
    }

    struct BindingBuilder
    {
        BindingBuilder(MaterialClassInfo & info):mInfo(info){}

        BindingBuilder & begin(const std::string & name)
        {
            assert(mInfo.bindings.find(name) == mInfo.bindings.end());
            bindingName = name;
            open = true;
            return *this;
        }

        BindingBuilder & stage(PipelineStage st)
        {
            cacheBinding.stage = st;
            return *this;
        }

        BindingBuilder & type(PipelineResourceType t)
        {
            cacheBinding.type = t;
            return *this;
        };

        BindingBuilder & shared(SharedResource shared)
        {
            cacheBinding.shared = shared;
            return *this;
        }

        BindingBuilder & size(u32 s)
        {
            cacheBinding.size = s;
            return *this;
        }

        BindingBuilder & push()
        {
            assert(open);
            mInfo.bindings.insert({bindingName,cacheBinding});
            open = false;
            cacheBinding = MaterialPipelineBinding();
            return *this;
        }

        MaterialClassInfo & end()
        {
            mInfo.GenerateKey();
            return mInfo;
        }

        private:
        bool open = false;      
        std::string bindingName;
        MaterialClassInfo & mInfo;
        MaterialPipelineBinding cacheBinding;
    };

    void MaterialManager::Initialize()
    {
        auto & container = mMaterialClassInfoMap;

        {
            auto builder = BindingBuilder(RegisterMaterial<PhysicalMaterial>(container));
        }

        {
            auto builder = BindingBuilder(RegisterMaterial<ToyMaterial>(container));
            
            builder.begin(u_global_scene_data)
            .type(PipelineResourceType::UniformBuffer)
            .size(sizeof(SceneUniform))
            .shared(SharedResource::Global)
            .stage(PipelineStage::VertexFragment)
            .push();

            auto info = builder.end();

            //temp
            auto core = static_cast<vulkan::VulkanCore*>(mpRenderer->GetCore());
            if(core)
            {
                core->create_pipeline(info);
            }

            ToyMaterial::materialInfo = info;
        }
    }
}