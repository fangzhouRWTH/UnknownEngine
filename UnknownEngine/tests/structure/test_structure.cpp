#include "catch2/catch_test_macros.hpp"
#include "core/structure/graph.hpp"
#include "platform/type.hpp"
#include <string>
#include <iostream>

using namespace unknown::structure;

TEST_CASE("STRUCTURE-DAG", "[standard]") {
  DirectAcyclicGraph<std::string, std::string> graph;
  auto n1 = graph.add_node("node1");
  auto n2 = graph.add_node("node2");
  auto n3 = graph.add_node("node3");
  auto n4 = graph.add_node("node4");
  auto n5 = graph.add_node("node5");
  auto n6 = graph.add_node("node6");
  auto n7 = graph.add_node("node7");

  auto e1 = graph.add_edge(n1, n2, "edge1");
  auto e2 = graph.add_edge(n1, n3, "edge2");
  auto e3 = graph.add_edge(n1, n4, "edge3");
  auto e4 = graph.add_edge(n1, n5, "edge4");
  auto e5 = graph.add_edge(n2, n3, "edge5");
  auto e6 = graph.add_edge(n3, n5, "edge6");
  auto e7 = graph.add_edge(n5, n6, "edge7");
  auto e8 = graph.add_edge(n6, n7, "edge8");
  auto e9 = graph.add_edge(n1, n7, "edge9");
  

  graph.remove_node(n1);

  // auto sort = graph.topo_sort();
  // for(auto n : sort)
  // {
  //   std::cout<<n<<std::endl;
  // }

  // bool isAcyc = graph.is_acyclic();
  // REQUIRE(isAcyc==true);

  // DirectedAcyclicGraph<u32,u32> graph2;
  // auto g2e1 = graph2.add_node(99);
  // auto g2e2 = graph2.add_node(99);
  // graph2.add_edge(g2e1,g2e2,999);
  // graph2.add_edge(g2e2,g2e1,999);
  // isAcyc = graph2.is_acyclic();
  // REQUIRE(isAcyc==false);
  // graph.add_edge(n6,n3,"???");
  // isAcyc = graph.is_acyclic();
  // REQUIRE(isAcyc==false);
}

// TEST_CASE("STRUCTURE-SIMPLE-GRAPH-ADD-LINK", "[standard]")
// {
//     SimpleGraph<1024u,16u> g;
//     auto n1 = g.AddNode();
//     auto n2 = g.AddNode();
//     auto n3 = g.AddNode();
//     auto n4 = g.AddNode();

//     REQUIRE(n1==0u);
//     REQUIRE(n2==1u);
//     REQUIRE(n3==2u);
//     REQUIRE(n4==3u);

//     g.Link(n1,n2);
//     g.Link(n1,n3);
//     g.Link(n3,n4);
//     g.Link(n1,n4);

//     std::vector<NodeIndex> childs;
//     std::vector<NodeIndex> parents;

//     g.GetChilds(n1,childs);
//     g.GetParents(n1, parents);
//     REQUIRE(childs.size()==3);
//     REQUIRE(childs[0] == n2);
//     REQUIRE(childs[1] == n3);
//     REQUIRE(childs[2] == n4);
//     REQUIRE(parents.size()==0);

//     g.GetChilds(n2,childs);
//     g.GetParents(n2, parents);
//     REQUIRE(childs.size()==0);
//     REQUIRE(parents.size()==1);
//     REQUIRE(parents[0] == n1);

//     g.GetChilds(n3,childs);
//     g.GetParents(n3, parents);
//     REQUIRE(childs.size()==1);
//     REQUIRE(childs[0] == n4);
//     REQUIRE(parents.size()==1);
//     REQUIRE(parents[0] == n1);

//     g.GetChilds(n4,childs);
//     g.GetParents(n4, parents);
//     REQUIRE(childs.size()==0);
//     REQUIRE(parents.size()==2);
//     REQUIRE(parents[0] == n3);
//     REQUIRE(parents[1] == n1);
// }

// TEST_CASE("STRUCTURE-SIMPLE-GRAPH-ADD-LINK-REMOVE", "[standard]")
// {
//     SimpleGraph<1024u,16u> g;
//     auto n1 = g.AddNode();
//     auto n2 = g.AddNode();
//     auto n3 = g.AddNode();
//     auto n4 = g.AddNode();

//     g.Link(n1,n2);
//     g.Link(n2,n3);
//     g.Link(n2,n4);
//     g.Link(n3,n4);

//     g.RemoveNode(n2);

//     std::vector<NodeIndex> childs;
//     std::vector<NodeIndex> parents;

//     REQUIRE(g.GetChilds(n1, childs));
//     REQUIRE(childs.size()==0);

//     REQUIRE(!g.GetChilds(n2, childs));
//     REQUIRE(childs.size()==0);

//     REQUIRE(g.GetChilds(n3, childs));
//     REQUIRE(g.GetParents(n3, parents));
//     REQUIRE(childs.size()==1);
//     REQUIRE(childs[0]==n4);
//     REQUIRE(parents.size()==0);

//     REQUIRE(g.GetChilds(n4, childs));
//     REQUIRE(g.GetParents(n4, parents));
//     REQUIRE(childs.size()==0);
//     REQUIRE(parents.size()==1);
//     REQUIRE(parents[0]==n3);
// }

// TEST_CASE("STRUCTURE-SIMPLE-GRAPH-ROOT", "[standard]")
// {
//     SimpleGraph<1024u, 4u> g;

//     auto n1 = g.AddNode();
//     auto n2 = g.AddParent(n1);
//     auto n3 = g.AddChild(n2);

//     std::vector<NodeIndex> childs;
//     std::vector<NodeIndex> roots;

//     g.GetChilds(n2, childs);
//     g.GetRoots(roots);

//     REQUIRE(childs.size() == 2);
//     REQUIRE(childs[0] == n1);
//     REQUIRE(childs[1] == n3);
//     REQUIRE(roots.size() == 1);
//     REQUIRE(roots[0] == n2);

//     auto n4 = g.AddParent(n2);
//     g.GetRoots(roots);
//     REQUIRE(roots.size() == 1);
//     REQUIRE(roots[0] == n4);

//     g.RemoveNode(n2);
//     g.GetRoots(roots);
//     REQUIRE(roots.size() == 3);

//     std::vector<NodeIndex> res;
//     res.push_back(n1);
//     res.push_back(n3);
//     res.push_back(n4);

//     for(u32 i = 0u; i < 3; i++)
//     {
//         auto r = roots[i];
//         for(auto i = res.begin(); i < res.end(); i++)
//         {
//             if(r == *i)
//             {
//                 res.erase(i);
//                 break;
//             }
//         }
//     }
//     REQUIRE(res.size()==0);
//     // REQUIRE(roots[0] == n3);
//     // REQUIRE(roots[1] == n1);
//     // REQUIRE(roots[2] == n4);
// }

// TEST_CASE("STRUCTURE-SIMPLE-GRAPH-ACYCLIC", "[standard]")
// {
//     SimpleGraph<1024u, 4u> g;

//     auto n1 = g.AddNode();
//     auto n2 = g.AddParent(n1);
//     auto n3 = g.AddChild(n2);
//     g.Check();
//     REQUIRE(g.IsAcyclic() == true);

//     auto n4 = g.AddChild(n3);
//     g.Link(n4,n3);
//     g.Check();
//     REQUIRE(g.IsAcyclic() == false);

//     g.RemoveNode(n4);
//     g.Check();
//     REQUIRE(g.IsAcyclic() == true);

//     g.Link(n3,n2);
//     g.Check();
//     REQUIRE(g.IsAcyclic() == false);
// }