#include "asset/utils/assimpHelper.hpp"
#include "assimpHelper.hpp"
#include "asset/utils/utils.hpp"

#include "core/structure/graph.hpp"

namespace unknown::asset
{
    bool AssimpImporter::Import(const AssimpImportConfig &config)
    {
        reset();
        mConfig = config;

        if (config.debugOutput)
        {
            debug_print_asset_hierarchy(config.path);
        }

        mData.scene = mImporter.ReadFile(config.path, config.pps);

        if (!mData.scene || mData.scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !mData.scene->mRootNode)
        {
            INFO_LOG("ERROR::ASSIMP::{}", mImporter.GetErrorString());
            reset();
            return false;
        }

        mData.meshPP = mData.scene->mMeshes;
        mData.meshCount = mData.meshCount;

        return true;
    }

    void AssimpImporter::reset()
    {
        mData.reset();
    }
}
