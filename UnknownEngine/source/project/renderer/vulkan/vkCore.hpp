#pragma once
#include "vkTypes.hpp"
#include "platform/type.hpp"
#include "core/math.hpp"
#include "memory/resource.hpp"
#include "renderer/rendererHandles.hpp"
#include "asset/resourceManager.hpp"

#include "vkDescriptor.hpp"

// temp
#include "context/engineContext.hpp"
#include "asset/modelLoader.hpp"

#include <vector>
#include "renderer/vulkan/sdk/vk_mem_alloc.h"
#include <deque>
#include <functional>

namespace unknown::renderer::vulkan
{
    class VulkanCore;

    struct DeletionQueue
    {
        std::deque<std::function<void()>> deletors;
        void push_function(std::function<void()> &&function)
        {
            deletors.push_back(function);
        }

        void flush()
        {
            for (auto it = deletors.rbegin(); it != deletors.rend(); it++)
            {
                (*it)();
            }
            deletors.clear();
        }
    };

    struct FrameData
    {
        VkSemaphore _swapchainSemaphore, _renderSemaphore;
        VkFence _renderFence;

        VkCommandPool _commandPool;
        VkCommandBuffer _mainCommandBuffer;

        DeletionQueue _deletionQueue;
        // cleaned each frame (being updated)
        DescriptorAllocatorGrowable _frameDescriptors;
    };

    constexpr u32 FRAME_OVERLAP = 2u;

    struct ComputePushConstants
    {
        Vec4f data1 = Vec4f(0.f, 0.f, 0.f, 0.f);
        Vec4f data2 = Vec4f(0.f, 0.f, 0.f, 0.f);
        Vec4f data3 = Vec4f(0.f, 0.f, 0.f, 0.f);
        Vec4f data4 = Vec4f(0.f, 0.f, 0.f, 0.f);
    };

    struct ComputeEffect
    {
        const char *name;
        VkPipeline pipeline;
        VkPipelineLayout layout;

        ComputePushConstants data;
    };

    // default pipelines data
    struct GLTFMetallic_Roughness
    {
        struct MaterialConstants
        {
            Vec4f colorFactors;
            Vec4f metal_rough_factors;
            // padding, we need it anyway for uniform buffers
            Vec4f extra[14];
        };

        struct MaterialResources
        {
            AllocatedImage colorImage;
            VkSampler colorSampler;
            AllocatedImage metalRoughImage;
            VkSampler metalRoughSampler;
            VkBuffer dataBuffer;
            uint32_t dataBufferOffset;
        };

        MaterialPipeline opaquePipeline;
        MaterialPipeline transparentPipeline;

        VkDescriptorSetLayout materialLayout;

        DescriptorWriter writer;

        void build_pipelines(VulkanCore *engine);
        void clear_resources(VkDevice device);

        MaterialInstance write_material(VkDevice device, MaterialPass pass, const MaterialResources &resources, DescriptorAllocatorGrowable &descriptorAllocator);
    };

    //> renderobject
    struct VulkanRenderObject
    {
        uint32_t indexCount;
        uint32_t firstIndex;
        VkBuffer indexBuffer;

        MaterialInstance *material;

        Mat4f transform;
        VkDeviceAddress vertexBufferAddress;
    };

    struct DrawContext
    {
        std::vector<VulkanRenderObject> OpaqueSurfaces;
    };
    //< renderobject

    //> meshnode
    // struct MeshNode : public Node
    // {

    //     std::shared_ptr<MeshAsset> mesh;

    //     virtual void Draw(const glm::mat4 &topMatrix, DrawContext &ctx) override;
    // };
    //< meshnode

    struct GPUMeshInfo
    {
        h64 meshDataHandle;
        asset::GeoSurface surface;
        GPUMeshBuffers meshBuffer;
    };

    struct MeshBufferBank
    {
        std::unordered_map<h64,GPUMeshInfo> meshInfos;
    };

    class VulkanCore
    {
    public:
        bool _isInitialized{false};
        i32 _frameNumber{0};

        VkExtent2D _windowExtent{1920, 1080};

        void *_windowPtr;
        //?window

        VkInstance _instance;
        VkDebugUtilsMessengerEXT _debug_messenger;
        VkPhysicalDevice _chosenGPU;
        VkDevice _device;

        FrameData _frames[FRAME_OVERLAP];

        FrameData &get_current_frame() { return _frames[_frameNumber % FRAME_OVERLAP]; };

        VkQueue _graphicsQueue;
        u32 _graphicsQueueFamily;

        VkSurfaceKHR _surface;
        VkSwapchainKHR _swapchain;
        VkFormat _swapchainImageFormat;
        VkExtent2D _swapchainExtent;
        VkExtent2D _drawExtent;
        float renderScale = 1.f;
        DescriptorAllocatorGrowable globalDescriptorAllocator;

        VkPipeline _gradientPipeline;
        VkPipelineLayout _gradientPipelineLayout;

        std::vector<VkFramebuffer> _framebuffers;
        std::vector<VkImage> _swapchainImages;
        std::vector<VkImageView> _swapchainImageViews;

        // DescriptorAllocator globalDescriptorAllocator;

        VkDescriptorSet _drawImageDescriptors;
        VkDescriptorSetLayout _drawImageDescriptorLayout;
        VkDescriptorSetLayout _singleImageDescriptorLayout;

        DeletionQueue _mainDeletionQueue;

        VmaAllocator _allocator;

        VkPipelineLayout _trianglePipelineLayout;
        VkPipeline _trianglePipeline;

        VkPipelineLayout _meshPipelineLayout;
        VkPipeline _meshPipeline;

        GPUMeshBuffers rectangle;
        std::vector<std::shared_ptr<asset::MeshAsset>> testMeshes;

        // immediate submit structures
        VkFence _immFence;
        VkCommandBuffer _immCommandBuffer;
        VkCommandPool _immCommandPool;

        AllocatedImage _whiteImage;
        AllocatedImage _blackImage;
        AllocatedImage _greyImage;
        AllocatedImage _errorCheckerboardImage;

        VkSampler _defaultSamplerLinear;
        VkSampler _defaultSamplerNearest;

        // draw resources
        DrawContext mainDrawContext;
        GPUSceneData sceneData;
        MaterialInstance defaultData;

        GLTFMetallic_Roughness metalRoughMaterial;

        AllocatedImage _drawImage;
        AllocatedImage _depthImage;

        // std::unordered_map<std::string, std::shared_ptr<Node>> loadedNodes;

        std::vector<ComputeEffect> backgroundEffects;
        int32_t currentBackgroundEffect{0};

        // material stuff
        VkDescriptorSetLayout _gpuSceneDataDescriptorLayout;
        VkDescriptorSetLayout _gltfMatDescriptorLayout;

        VkPipeline _gltfDefaultOpaque;
        VkPipeline _gltfDefaultTranslucent;
        VkPipelineLayout _gltfPipelineLayout;

        void init(void *windowPtr);

        void cleanup();

        void draw(u32 width, u32 height, const EngineContext &context);

        void push_dynamic_render_object(VulkanRenderObject renderObject);

        std::vector<ComputeEffect> &test_get_backgroud_effects() { return backgroundEffects; }

        i32 &test_get_backgroud_effect_index() { return currentBackgroundEffect; }

        void test_try_resize_swapchain(u32 width, u32 height)
        {
            if (resize_requested)
            {
                resize_requested = false;
                resize_swapchain(width, height);
            }
        }
        void draw_background(VkCommandBuffer cmd);
        void draw_geometry(VkCommandBuffer cmd, const EngineContext &context);

        void test_update_scene(const EngineContext &context);

        void immediate_submit(std::function<void(VkCommandBuffer cmd)> &&function);

        GPUMeshBufferHandle createGPUMesh(std::span<uint32_t> indices, std::span<Vertex> vertices);
        GPUMeshBuffers uploadMesh(std::span<uint32_t> indices, std::span<Vertex> vertices);

        AllocatedBuffer create_buffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);
        AllocatedImage create_image(VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped = false);
        AllocatedImage create_image(void *data, VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped = false);

        void destroy_buffer(const AllocatedBuffer &buffer);
        void destroy_image(const AllocatedImage& img);

        bool resize_requested{false};
        bool freeze_rendering{false};

        // bool stop_rendering{false};
    private:
        void init_vulkan();

        void init_swapchain();
        void create_swapchain(u32 width, u32 height);
        void destroy_swapchain();
        void resize_swapchain(u32 width, u32 height);

        void init_commands();

        void init_background_pipelines();

        void init_pipelines();

        void init_triangle_pipeline();
        void init_mesh_pipeline();

        void init_descriptors();

        void init_sync_structures();

        // temp
        void init_default_data();

    private:
        MeshBufferBank mMeshBufferBank;
        //ResourceArray<GPUMeshBufferHandle, GPUMeshBuffers> mGPUMeshBuffersMap;
    };
}