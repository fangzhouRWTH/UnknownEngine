#include "asset/utils/assimpHelper.hpp"
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
            //debug_print_asset_hierarchy(config.path);
        }

        mData.scene = mImporter.ReadFile(config.path, config.postProcessOptions);

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

    bool AssimpImporter::LoadSceneTree(std::shared_ptr<SceneTree> sceneTree)
    {
        auto scene = mData.scene;
        auto root = scene->mRootNode;
        u32 childNum = root->mNumChildren;
        auto children = root->mChildren;

        for(u32 i = 0u; i < childNum; i++)
        {
            auto child = children[i];
            load_node(scene,child,sceneTree,sceneTree->RootIndex(), mConfig);
        }


        //test
        if(scene->HasTextures())
        {
            auto texNum = scene->mNumTextures;
            for(u32 i = 0u; i < texNum; i++)
            {
                auto tx = scene->mTextures[i];
                debug_print_texture_info(tx);
                //tx->pcData
            }
        }

        if(scene->HasMaterials())
        {
            //test
            auto matNum = scene->mNumMaterials;
            auto mat = scene->mMaterials[0];
            //mat->
        }

        return false;
    }

    void AssimpImporter::reset()
    {
        mData.reset();
    }
}
