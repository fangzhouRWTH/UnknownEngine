#include "ecs/component.hpp"
#include "asset/asset.hpp"

namespace unknown::ecs
{
    struct CMesh : public Component<CMesh>
    {
        CMesh(){}
        //CMesh(asset::LoadedRenderObject renderObj): renderObject(renderObj) {}
        //asset::LoadedRenderObject renderObject;
    };
}