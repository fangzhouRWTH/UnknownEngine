#include "core/handles.hpp"
#include "memory/resource.hpp"
#include "catch2/catch_test_macros.hpp"

using namespace unknown;

struct TestHandle : public HandleTemplate<TestHandle>
{

};

struct TestData
{
    u32 num = 0u;
};

TEST_CASE("HANDLE", "[standard]")
{
    ResourceTable<TestHandle,TestData> table;
    TestHandle h1 = table.Create();
    TestHandle h2 = table.Create();
    TestData data1; data1.num = 111u;
    TestData res1;
    TestData data2; data2.num = 222u;
    TestData res2;
    //table.Set(h1,data1);
    *table.Get(h1) = data1;
    *table.Get(h2) = data2;
    res1 = *table.Get(h1);
    table.Remove(h2);
    REQUIRE(table.Count() == 1u);
    
    TestHandle h3 = table.Create();
    *table.Get(h3) = data2;

    REQUIRE(res1.num==111u);
    REQUIRE(!table.Get(h2));
    REQUIRE(h3.value()==2u);
    res2 = *table.Get(h3);
    REQUIRE(res2.num==222u);
    REQUIRE(table.KeySize() == 3u);
    REQUIRE(table.Size() == 2u);
    REQUIRE(table.Count() == 2u);
}
