#include "vulkan_renderer/memAllocation.hpp"
#include "catch2/catch_test_macros.hpp"
#include <iostream>
#include <string>
using namespace unknown::renderer::vulkan;

void writeChar(void * ptr, u32 size, char c)
{
    memset(ptr,c,size);
}

void printMem(LinearMemory & mem)
{
    auto ptr = reinterpret_cast<char*>(mem.getBasePtr());
    auto size = mem.getOffset();
    std::string out;
    for(auto i = 0; i < size; i++)
    
    {
        out.push_back(*(ptr + i));
    }

    std::cout << out << std::endl;
}

TEST_CASE("MEMORY", "[standard]") {
    constexpr u32 poolSize = 16u * MB;
    constexpr u32 maxAlignment = 16u;

    LinearMemory mem(poolSize, maxAlignment);
    MemoryAllocation a1 = mem.allocate(78);
    MemoryAllocation a2 = mem.allocate(55);
    MemoryAllocation a3 = mem.allocate(33);
    REQUIRE(a1.ptr == mem.getBasePtr());
    REQUIRE(a1.reserve == 80);
    REQUIRE(a2.ptr == (reinterpret_cast<u8*>(mem.getBasePtr())+80));
    REQUIRE(a2.reserve == 64);
    REQUIRE(a3.ptr == (reinterpret_cast<u8*>(mem.getBasePtr())+144));
    REQUIRE(a3.reserve == 48);
    mem.reset();

    MemoryAllocation a4 = mem.allocate(55);
    REQUIRE(a4.ptr == mem.getBasePtr());
    REQUIRE(a4.reserve == 64);
    writeChar(a4.ptr,a4.size,'o');

    MemoryAllocation a5 = mem.allocate(6);
    REQUIRE(a5.ptr == (reinterpret_cast<u8*>(mem.getBasePtr())+64));
    REQUIRE(a5.reserve == 16);
    writeChar(a5.ptr,a5.size,'v');

    LinearMemory subMme(mem,64,16u);
    auto sub_a1 = subMme.allocate(4);
    writeChar(sub_a1.ptr,sub_a1.size,'a');

    printMem(mem);
}