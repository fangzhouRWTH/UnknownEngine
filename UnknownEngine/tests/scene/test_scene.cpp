#include "catch2/catch_test_macros.hpp"
#include "core/structure/graph.hpp"
#include "platform/type.hpp"
#include "world/scene.hpp"

using namespace unknown;

TEST_CASE("SCENE", "[standard]")
{
    SceneTree tree;

    auto p1 = tree.CreateAttachRoot(SceneNodeType::Mesh);
    REQUIRE(p1.first == 1);
    REQUIRE(p1.second != nullptr);

    auto p2 = tree.CreateAttachRoot(SceneNodeType::Empty);
    auto p3 = tree.CreateNode(SceneNodeType::Mesh,p2.first);
    auto p4 = tree.CreateNode(SceneNodeType::Mesh,p2.first);
    tree.RemoveNode(p2.first);

    REQUIRE(tree.HasNode(p2.first)==false);
    SceneNodeIndex idx;
    idx = tree.GetParent(p3.first);
    REQUIRE(idx == 0);
    idx = tree.GetParent(p4.first);
    REQUIRE(idx == 0);

    std::vector<SceneNodeIndex> childs;
    tree.GetChilds(0,childs);
    REQUIRE(childs.size()==3);
    REQUIRE(childs[0]==p1.first);
    REQUIRE(childs[1]==p3.first);
    REQUIRE(childs[2]==p4.first);
}