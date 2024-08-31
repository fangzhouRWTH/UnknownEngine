#pragma once
#include <nlohmann/json.hpp>
#include <memory>
#include <string>
#include <fstream>
#include <iostream>
#include "core/math.hpp"
#include "core/structure.hpp"
#include "renderer/rendererHandles.hpp"

namespace unknown
{
    // struct SceneObjectsTree
    // {
    // public:
    //     structure::NodeTree<std::string, SceneNode> sceneTree;

    // private:
    // };

    class SceneLoader
    {
    public:
        static void LoadScene(std::string sceneFilePath)
        {
            // std::ifstream f(sceneFilePath);
            // json data = json::parse(f);

            // auto scene = data["scene"];
            // auto entities = scene.find("entities");
            // if (entities != scene.end())
            // {
            //     for (auto it = entities->begin(); it != entities->end(); it++)
            //     {
            //         auto name = it->find("name");
            //         if (name != it->end())
            //         {
            //             std::cout << name.value() << std::endl;
            //         }
            //     }
            // }
        }
    };
}