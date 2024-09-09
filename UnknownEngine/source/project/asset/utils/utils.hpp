#pragma once

#include "assimpHelper.hpp"
#include "debug/log.hpp"
#include "configuration/globalValues.hpp"

#include <iostream>
#include <fstream>

namespace unknown::asset
{
    static void debug_scene_process_node(std::ofstream &ofs, const aiScene *scene, aiNode *node, u32 indent)
    {
        for (auto i = 0u; i < indent; i++)
        {
            ofs << "--";
        }
        ofs << "[node] [level " << indent << "]: " << node->mName.C_Str() << "\n";

        u32 cNum = node->mNumChildren;
        u32 mNum = node->mNumMeshes;

        for (u32 i = 0u; i < mNum; i++)
        {
            for (auto j = 0u; j < indent + 1u; j++)
            {
                ofs << "--";
            }
            auto m = node->mMeshes[i];

            ofs << "[mesh] [index " << node->mMeshes[i] << "]: " << scene->mMeshes[node->mMeshes[i]]->mName.C_Str() << "\n";
        }

        for (u32 i = 0u; i < cNum; i++)
        {
            auto c = node->mChildren[i];
            debug_scene_process_node(ofs, scene, c, indent + 1u);
        }
    }

    static void debug_scene_process_mesh(std::ofstream &ofs, aiNode *node, u32 indent)
    {
    }

    static void debug_scene_graph_export(std::string_view assetPath, std::string_view outputPath)
    {
        std::ofstream file(outputPath.data());
        if (file.bad())
            return;

        u32 indent = 0u;
        Assimp::Importer import;
        const aiScene *scene = import.ReadFile(assetPath.data(), aiProcess_Triangulate | aiProcess_FlipUVs);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            INFO_LOG("ERROR::ASSIMP::{}", import.GetErrorString());
            return;
        }

        file << "Asset: [" << scene->mName.C_Str() << "]:[" << assetPath << "]" << "\n";

        debug_scene_process_node(file, scene, scene->mRootNode, indent + 1u);

        file.close();
    }

    static void debug_print_asset_hierarchy(std::string_view assetPath)
    {
        std::string output = config::log_folder_path + "asset_manager_log.txt";
        debug_scene_graph_export(assetPath, output);
    }
}