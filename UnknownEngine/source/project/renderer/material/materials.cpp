#include "materials.hpp"

namespace unknown::renderer
{
    MaterialClassID IMaterial::current_id = 0u;
    std::mutex IMaterial::mLock;

    IMaterial::IMaterial(){}
    
    MaterialClassID IMaterial::getUniqueID()
    {
        assert(current_id != 0xFFFFFFFF);
        std::lock_guard<std::mutex> guard(mLock);
        MaterialClassID id = current_id++;
        return id;
    }

    const std::string ToyMaterial::vertexShaderName = "common.vert.spv";
    const std::string ToyMaterial::fragmentShaderName ="toy.frag.spv";

    const std::string PhysicalMaterial::vertexShaderName = "common.vert.spv";
    const std::string PhysicalMaterial::fragmentShaderName ="toy.frag.spv";

    const std::string PhongMaterial::vertexShaderName = "common.vert.spv";
    const std::string PhongMaterial::fragmentShaderName ="toy.frag.spv";
}