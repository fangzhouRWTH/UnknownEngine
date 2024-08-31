#include "catch2/catch_test_macros.hpp"
#include "core/structure.hpp"
#include <string>
#include <iostream>

using namespace unknown;
using namespace ecs;

TEST_CASE("TEST NODE GRAPH", "[standard]")
{
    structure::NodeGraph<std::string,u32> graph;

    graph.AddNode("a",0u);
    graph.AddNode("b",1u);
    graph.AddNode("c",2u);
    graph.AddNode("x",3u);
    graph.AddNode("y",4u);
    graph.AddNode("s1",5u);
    graph.AddNode("s2",6u);

    graph.AddEdge("a","b");
    graph.AddEdge("b","c");
    graph.AddEdge("a","x");
    graph.AddEdge("x","y");
    //graph.AddEdge("y","a");
    graph.AddEdge("y","b");

    auto bRes = graph.Build();
    auto res = graph.GetTopologicKeyOrder();
    //std::cout<"size : "<<
    for(auto r : res)
    {
        std::cout <<"[" << r << "]";
    }
    std::cout<<std::endl;
    REQUIRE(bRes);
}