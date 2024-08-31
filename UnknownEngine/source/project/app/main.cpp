#include "engine.hpp"
#include "vulkan/vulkan.hpp"
#include "iostream"

int main()
{
    //uint32_t extensionCount = 0;
    //vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    //std::cout << extensionCount << std::endl;
    unknown::Engine engine;

    engine.Initialize();
    engine.Run();
    engine.Shutdown();

    return 0;
}