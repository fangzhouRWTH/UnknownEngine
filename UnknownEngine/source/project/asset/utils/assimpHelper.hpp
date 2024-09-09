#pragma once

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "platform/type.hpp"

#include <string>

namespace unknown::asset
{
    struct AssimpImportConfig
    {
        std::string path;
        //TODO wrap
        u32 pps = aiPostProcessSteps::aiProcess_Triangulate | aiPostProcessSteps::aiProcess_FlipUVs;
        bool debugOutput = false;
    };

    struct AssimpImportData
    {
        const aiScene* scene = nullptr;
        u32 meshCount = 0u;
        aiMesh ** meshPP = nullptr;

        void reset()
        {
            scene = nullptr;
            meshCount = 0u;
            meshPP = nullptr;
        }
    };

    class AssimpImporter
    {
    public:
        AssimpImporter() {};
        bool Import(const AssimpImportConfig &config);

    private:
        void reset();

        AssimpImportConfig mConfig;
        Assimp::Importer mImporter;

        AssimpImportData mData;
    };
}