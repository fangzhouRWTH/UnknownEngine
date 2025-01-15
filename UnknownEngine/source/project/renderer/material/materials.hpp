#pragma once
#include <cassert>
#include <string>
#include <mutex>

#include "platform/type.hpp"
#include "renderer/api_interface.hpp"

namespace unknown::renderer
{
    class MaterialManager;

    class IMaterial
    {
        public:
            IMaterial();
            virtual ~IMaterial(){}
        private:
            static MaterialClassID current_id;
            static std::mutex mLock;
        protected:
            static MaterialClassID getUniqueID();
    };

    template<typename M>
    class MaterialBase : public IMaterial
    {
            friend class MaterialManager;
        public:
            static u32 GetMaterialClassID()
            {
                return unique_id;
            }

            // MaterialKey GetKey()
            // {
            //     //todo
            //     MaterialKey = MaterialKey(unique_id);
            //     return MaterialKey;
            // }

            static std::string_view GetVertexShaderName()
            {
                return vertexShaderName;
            }

            static std::string_view GetFragmentShaderName()
            {
                return fragmentShaderName;
            }

            const static MaterialClassInfo & GetMaterialInfo()
            {
                return materialInfo;
            }

        protected:
            MaterialBase()
            {
                // u32 id;
                // assert(getUniqueID(id));
                // static u32 unique_id = id;
            };
            virtual ~MaterialBase(){}
            const static MaterialClassID unique_id;
            
            const static std::string vertexShaderName;
            const static std::string fragmentShaderName;

            static MaterialClassInfo materialInfo;
    };

    template<typename M>
    const MaterialClassID MaterialBase<M>::unique_id = IMaterial::getUniqueID();

    template<typename M>
    MaterialClassInfo MaterialBase<M>::materialInfo;

    //Currently only supports static material(pipeline states)
    //TODO: Make material pipeline be built during runtime ( VK TARGET ) 
    class ToyMaterial : public MaterialBase<ToyMaterial>
    {
        public:
            ToyMaterial(){}
        private:
    };

    class PhysicalMaterial : public MaterialBase<PhysicalMaterial>
    {
        public:
            PhysicalMaterial(){}
        private:
    };

    class PhongMaterial : public MaterialBase<PhongMaterial>
    {
        public:
            PhongMaterial(){}
        private:
    };
}