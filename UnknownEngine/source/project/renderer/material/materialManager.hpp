#pragma once

#include "materials.hpp"
#include "renderer/api_interface.hpp"

#include <unordered_map>
#include <memory>
#include <string>
//#include <vector>

namespace unknown::renderer
{
    class IRenderer;

    class IMaterialManager
    {
        public:
        virtual void Initialize() = 0;
    };

    typedef std::unordered_map<MaterialClassID,MaterialClassInfo> MaterialClassMap;

    class MaterialManager:public IMaterialManager
    {
        public:
            MaterialManager(std::shared_ptr<IRenderer> renderer):mpRenderer(renderer) {};
            ~MaterialManager(){};

            virtual void Initialize() override;
            //void RegisterMaterial

        private:
            std::shared_ptr<IRenderer> mpRenderer; 
            MaterialClassMap mMaterialClassInfoMap;

    };
}