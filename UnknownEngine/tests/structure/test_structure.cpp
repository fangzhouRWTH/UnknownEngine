#include "catch2/catch_test_macros.hpp"
#include "core/structure/graph.hpp"
#include "platform/type.hpp"

using namespace unknown::structure;

TEST_CASE("STRUCTURE-SIMPLE-GRAPH-ADD-LINK", "[standard]")
{
    SimpleGraph<1024u,16u> g;
    auto n1 = g.AddNode();
    auto n2 = g.AddNode();
    auto n3 = g.AddNode();
    auto n4 = g.AddNode();

    REQUIRE(n1==0u);
    REQUIRE(n2==1u);
    REQUIRE(n3==2u);
    REQUIRE(n4==3u);

    g.Link(n1,n2);
    g.Link(n1,n3);
    g.Link(n3,n4);
    g.Link(n1,n4);

    std::vector<NodeIndex> childs;
    std::vector<NodeIndex> parents;

    g.GetChilds(n1,childs);
    g.GetParents(n1, parents);
    REQUIRE(childs.size()==3);
    REQUIRE(childs[0] == n2);
    REQUIRE(childs[1] == n3);
    REQUIRE(childs[2] == n4);
    REQUIRE(parents.size()==0);

    g.GetChilds(n2,childs);
    g.GetParents(n2, parents);
    REQUIRE(childs.size()==0);
    REQUIRE(parents.size()==1);
    REQUIRE(parents[0] == n1);

    g.GetChilds(n3,childs);
    g.GetParents(n3, parents);
    REQUIRE(childs.size()==1);
    REQUIRE(childs[0] == n4);
    REQUIRE(parents.size()==1);
    REQUIRE(parents[0] == n1);

    g.GetChilds(n4,childs);
    g.GetParents(n4, parents);
    REQUIRE(childs.size()==0);
    REQUIRE(parents.size()==2);
    REQUIRE(parents[0] == n3);
    REQUIRE(parents[1] == n1);
}

TEST_CASE("STRUCTURE-SIMPLE-GRAPH-ADD-LINK-REMOVE", "[standard]")
{
    SimpleGraph<1024u,16u> g;
    auto n1 = g.AddNode();
    auto n2 = g.AddNode();
    auto n3 = g.AddNode();
    auto n4 = g.AddNode();

    g.Link(n1,n2);
    g.Link(n2,n3);
    g.Link(n2,n4);
    g.Link(n3,n4);

    g.RemoveNode(n2);

    std::vector<NodeIndex> childs;
    std::vector<NodeIndex> parents;

    REQUIRE(g.GetChilds(n1, childs));
    REQUIRE(childs.size()==0);

    REQUIRE(!g.GetChilds(n2, childs));
    REQUIRE(childs.size()==0);

    REQUIRE(g.GetChilds(n3, childs));
    REQUIRE(g.GetParents(n3, parents));
    REQUIRE(childs.size()==1);
    REQUIRE(childs[0]==n4);
    REQUIRE(parents.size()==0);

    REQUIRE(g.GetChilds(n4, childs));
    REQUIRE(g.GetParents(n4, parents));
    REQUIRE(childs.size()==0);
    REQUIRE(parents.size()==1);
    REQUIRE(parents[0]==n3);
}

TEST_CASE("STRUCTURE-SIMPLE-GRAPH-ROOT", "[standard]")
{
    SimpleGraph<1024u, 4u> g;

    auto n1 = g.AddNode();
    auto n2 = g.AddParent(n1);
    auto n3 = g.AddChild(n2);

    std::vector<NodeIndex> childs;
    std::vector<NodeIndex> roots;

    g.GetChilds(n2, childs);
    g.GetRoots(roots);

    REQUIRE(childs.size() == 2);
    REQUIRE(childs[0] == n1);
    REQUIRE(childs[1] == n3);
    REQUIRE(roots.size() == 1);
    REQUIRE(roots[0] == n2);

    auto n4 = g.AddParent(n2);
    g.GetRoots(roots);
    REQUIRE(roots.size() == 1);
    REQUIRE(roots[0] == n4);


    g.RemoveNode(n2);
    g.GetRoots(roots);
    REQUIRE(roots.size() == 3);

    std::vector<NodeIndex> res;
    res.push_back(n1);
    res.push_back(n3);
    res.push_back(n4);

    for(u32 i = 0u; i < 3; i++)
    {
        auto r = roots[i];
        for(auto i = res.begin(); i < res.end(); i++)
        {
            if(r == *i)
            {
                res.erase(i);
                break;
            }
        }
    }
    REQUIRE(res.size()==0);
    // REQUIRE(roots[0] == n3);
    // REQUIRE(roots[1] == n1);
    // REQUIRE(roots[2] == n4);
}

TEST_CASE("STRUCTURE-SIMPLE-GRAPH-ACYCLIC", "[standard]")
{
    SimpleGraph<1024u, 4u> g;

    auto n1 = g.AddNode();
    auto n2 = g.AddParent(n1);
    auto n3 = g.AddChild(n2);
    g.Check();
    REQUIRE(g.IsAcyclic() == true);

    auto n4 = g.AddChild(n3);
    g.Link(n4,n3);
    g.Check();
    REQUIRE(g.IsAcyclic() == false);

    g.RemoveNode(n4);
    g.Check();
    REQUIRE(g.IsAcyclic() == true);

    g.Link(n3,n2);
    g.Check();
    REQUIRE(g.IsAcyclic() == false);
}