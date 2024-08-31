#include "vkCore.hpp"
#include "configuration/globalValues.hpp"
#define USE_GLFW

#if defined(USE_GLFW)
#include "GLFW/glfw3.h"
#endif

#include "vkInitializer.hpp"
#include "vkUtils.hpp"
#include "vkPipeline.hpp"
#include "VkBootstrap.h"
#include "core/hash.hpp"

#include "asset/assetManager.hpp"

#include <array>
#include <iostream>
#include <fstream>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#include "renderer/gui/simpleGUI.hpp"

#define VMA_IMPLEMENTATION
#include "renderer/vulkan/sdk/vk_mem_alloc.h"

constexpr bool bUseValidationLayers = true;

namespace unknown::renderer::vulkan
{
    void VulkanCore::init(void *windowPtr)
    {
        float a = 0.f;
        _windowPtr = windowPtr;
        init_vulkan();
        init_swapchain();
        init_commands();
        init_sync_structures();
        init_descriptors();
        init_pipelines();
        // init_imgui(windowPtr, true);
        init_default_data();
        _isInitialized = true;
    }

    void VulkanCore::init_default_data()
    {
        std::array<asset::VkVertex, 4> rect_vertices;

        rect_vertices[0].position = {0.5, -0.5, 0};
        rect_vertices[1].position = {0.5, 0.5, 0};
        rect_vertices[2].position = {-0.5, -0.5, 0};
        rect_vertices[3].position = {-0.5, 0.5, 0};

        rect_vertices[0].color = {0, 0, 0, 1};
        rect_vertices[1].color = {0.5, 0.5, 0.5, 1};
        rect_vertices[2].color = {1, 0, 0, 1};
        rect_vertices[3].color = {0, 1, 0, 1};

        rect_vertices[0].uv_x = 1;
        rect_vertices[0].uv_y = 0;
        rect_vertices[1].uv_x = 0;
        rect_vertices[1].uv_y = 0;
        rect_vertices[2].uv_x = 1;
        rect_vertices[2].uv_y = 1;
        rect_vertices[3].uv_x = 0;
        rect_vertices[3].uv_y = 1;

        std::array<uint32_t, 6> rect_indices;

        rect_indices[0] = 0;
        rect_indices[1] = 1;
        rect_indices[2] = 2;

        rect_indices[3] = 2;
        rect_indices[4] = 1;
        rect_indices[5] = 3;

        // rectangle = uploadMesh(std::span{Suzanne_idx, Suzanne_idx_count}, std::span{Suzanne_vtx, Suzanne_vtx_count});

        asset::RenderElementAssetInfo assetInfo;
        assetInfo.path = config::model_folder_path + "test/three_boxes.glb";
        // assetInfo.path = config::model_folder_path + "test/right_hand_axis.fbx";
        // assetInfo.path = config::model_folder_path + "backpack/backpack.obj";

        // default image
        uint32_t white = math::pack_nml_4f_u32(Vec4f(1, 1, 1, 1));
        _whiteImage = create_image((void *)&white, VkExtent3D{1, 1, 1}, VK_FORMAT_R8G8B8A8_UNORM,
                                   VK_IMAGE_USAGE_SAMPLED_BIT);

        uint32_t grey = math::pack_nml_4f_u32(Vec4f(0.66f, 0.66f, 0.66f, 1));
        _greyImage = create_image((void *)&grey, VkExtent3D{1, 1, 1}, VK_FORMAT_R8G8B8A8_UNORM,
                                  VK_IMAGE_USAGE_SAMPLED_BIT);

        uint32_t black = math::pack_nml_4f_u32(Vec4f(0, 0, 0, 0));
        _blackImage = create_image((void *)&black, VkExtent3D{1, 1, 1}, VK_FORMAT_R8G8B8A8_UNORM,
                                   VK_IMAGE_USAGE_SAMPLED_BIT);

        // checkerboard
        uint32_t magenta = math::pack_nml_4f_u32(Vec4f(1, 0, 1, 1));
        std::array<uint32_t, 16 * 16> pixels; // for 16x16 checkerboard texture
        for (int x = 0; x < 16; x++)
        {
            for (int y = 0; y < 16; y++)
            {
                pixels[y * 16 + x] = ((x % 2) ^ (y % 2)) ? magenta : black;
            }
        }
        _errorCheckerboardImage = create_image(pixels.data(), VkExtent3D{16, 16, 1}, VK_FORMAT_R8G8B8A8_UNORM,
                                               VK_IMAGE_USAGE_SAMPLED_BIT);

        VkSamplerCreateInfo sampl = {.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};

        sampl.magFilter = VK_FILTER_NEAREST;
        sampl.minFilter = VK_FILTER_NEAREST;

        vkCreateSampler(_device, &sampl, nullptr, &_defaultSamplerNearest);

        sampl.magFilter = VK_FILTER_LINEAR;
        sampl.minFilter = VK_FILTER_LINEAR;
        vkCreateSampler(_device, &sampl, nullptr, &_defaultSamplerLinear);

        _mainDeletionQueue.push_function([&]()
                                         {
		vkDestroySampler(_device,_defaultSamplerNearest,nullptr);
		vkDestroySampler(_device,_defaultSamplerLinear,nullptr);

		destroy_image(_whiteImage);
		destroy_image(_greyImage);
		destroy_image(_blackImage);
		destroy_image(_errorCheckerboardImage); });
        //

        //
        GLTFMetallic_Roughness::MaterialResources materialResources;
        // default the material textures
        materialResources.colorImage = _whiteImage;
        materialResources.colorSampler = _defaultSamplerLinear;
        materialResources.metalRoughImage = _whiteImage;
        materialResources.metalRoughSampler = _defaultSamplerLinear;

        // set the uniform buffer for the material data
        AllocatedBuffer materialConstants = create_buffer(sizeof(GLTFMetallic_Roughness::MaterialConstants), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

        // write the buffer
        GLTFMetallic_Roughness::MaterialConstants *sceneUniformData = (GLTFMetallic_Roughness::MaterialConstants *)materialConstants.allocation->GetMappedData();
        sceneUniformData->colorFactors = Vec4f{1, 1, 1, 1};
        sceneUniformData->metal_rough_factors = Vec4f{1, 0.5, 0, 0};

        _mainDeletionQueue.push_function([=, this]()
                                         { destroy_buffer(materialConstants); });

        materialResources.dataBuffer = materialConstants.buffer;
        materialResources.dataBufferOffset = 0;

        defaultData = metalRoughMaterial.write_material(_device, MaterialPass::MainColor, materialResources, globalDescriptorAllocator);
        //

        // load meshes
        {
            mainDrawContext.OpaqueSurfaces.clear();
            auto rm = asset::ResourceManager::Get();
            std::string modelPath = "/home/fzl/workspace/git_projects/RenderEngineV0/UnknownEngine/assets/models/structure/structure.glb";
            // std::string modelPath = "/home/fzl/workspace/git_projects/RenderEngineV0/assets/models/test/three_boxes.glb";
            h64 h = math::HashString(modelPath);
            rm->AddResourceMetaData(modelPath, asset::ResourceType::Model);
            auto b = rm->LoadModelData(h);

            // test
            auto sd = rm->GetSceneData(h);

            auto op = [](std::shared_ptr<asset::ISceneContent> parent, std::shared_ptr<asset::ISceneContent> child) -> void
            {
                child->transform = parent->transform * child->transform;
                // INFO_LOG("update X : {}, Y : {}, Z : {}", child->transform.col(3).x(), child->transform.col(3).y(), child->transform.col(3).z())
            };
            sd->scene.RecursiveOperationDownward(op);

            auto nodes = sd->scene.GetTopologicContentOrder();
            // auto nkeys = sd->scene.GetTopologicKeyOrder();
            // for(auto k:nkeys)
            // {
            //     auto sceneMesh = sd->scene.GetNode(k)->content;

            // }

            u32 count = 0u;
            for (auto n : nodes)
            {
                INFO_LOG("X : {}, Y : {}, Z : {}", n->transform.col(3).x(), n->transform.col(3).y(), n->transform.col(3).z())
                if (n->type == asset::SceneContentType::Mesh)
                {
                    count++;
                    auto sceneMesh = std::dynamic_pointer_cast<asset::SceneMesh>(n);
                    assert(sceneMesh);
                    GPUMeshInfo meshInfo;
                    auto meshHandle = sceneMesh->meshDataHash;

                    if (mMeshBufferBank.meshInfos.find(meshHandle) != mMeshBufferBank.meshInfos.end())
                    {
                        sceneMesh->meshGpuInfoHash = meshHandle;
                    }
                    else
                    {
                        auto meshData = rm->GetMeshData(meshHandle);
                        meshInfo.meshDataHandle = meshHandle;
                        meshInfo.surface.startIndex = 0u;
                        meshInfo.surface.count = meshData->indices.size();
                        meshInfo.meshBuffer = uploadMesh(meshData->indices, meshData->vertices);
                        mMeshBufferBank.meshInfos.insert({meshInfo.meshDataHandle, meshInfo});
                        sceneMesh->meshGpuInfoHash = meshHandle;
                    }

                    Mat4f nodeMatrix = sceneMesh->transform;
                    RenderObject def;
                    def.indexCount = meshInfo.surface.count;
                    def.firstIndex = meshInfo.surface.startIndex;
                    def.indexBuffer = meshInfo.meshBuffer.indexBuffer.buffer;
                    def.material = &defaultData;

                    def.transform = nodeMatrix;
                    def.vertexBufferAddress = meshInfo.meshBuffer.vertexBufferAddress;

                    mainDrawContext.OpaqueSurfaces.push_back(def);
                }
            }
            // //asset::MeshRawData rData;
            // //asset::AssetManager::LoadMeshRawData(rData, assetInfo);
            // asset::SceneRawData rData;
            // asset::AssetManager::LoadSceneRawData(rData, assetInfo);

            // u32 meshCount = rData.subMeshIndicesRanges.size();
            // // testMeshes.resize(meshCount);
            // testMeshes.resize(1);
            // for (auto i = 0u; i < 1; i++)
            // {
            //     auto rangeI = rData.subMeshIndicesRanges[i];
            //     auto rangeV = rData.subMeshVerticesRanges[i];
            //     asset::MeshAsset newMesh;
            //     // asset::GeoSurface newSurface{rangeI.first,rangeI.second - rangeI.first};
            //     asset::GeoSurface newSurface{0, rData.indices.size()};
            //     newMesh.surfaces.resize(1);
            //     newMesh.surfaces[0] = newSurface;
            //     newMesh.meshBuffers = uploadMesh(
            //         rData.indices,
            //         rData.vertices);
            //     // newMesh.meshBuffers = uploadMesh(
            //     //     std::span<u32>(rData.indices.begin(),rangeI.second-rangeI.first),
            //     //     std::span<asset::VkVertex>(rData.vertices.begin(),rangeV.second - rangeV.first));

            //     testMeshes[0] = std::make_shared<asset::MeshAsset>(std::move(newMesh));
            //     auto &mesh = testMeshes[0];
            //     auto &s = mesh->surfaces[0];
            // temps
            // }

            // for (auto meshInfo : mMeshBufferBank.meshInfos)
            // {
            //     const auto &mesh = meshInfo.second;
            //     Mat4f nodeMatrix = Mat4f::Identity();
            //     RenderObject def;
            //     def.indexCount = mesh.surface.count;
            //     def.firstIndex = mesh.surface.startIndex;
            //     def.indexBuffer = mesh.meshBuffer.indexBuffer.buffer;
            //     def.material = &defaultData;

            //     def.transform = nodeMatrix;
            //     def.vertexBufferAddress = mesh.meshBuffer.vertexBufferAddress;

            //     mainDrawContext.OpaqueSurfaces.push_back(def);
            // }
            auto a = 123;
            a++;
        }
    }

    void VulkanCore::cleanup()
    {
        if (_isInitialized)
        {
            vkDeviceWaitIdle(_device);
            for (u32 i = 0; i < FRAME_OVERLAP; i++)
            {
                vkDestroyCommandPool(_device, _frames[i]._commandPool, nullptr);

                vkDestroyFence(_device, _frames[i]._renderFence, nullptr);
                vkDestroySemaphore(_device, _frames[i]._renderSemaphore, nullptr);
                vkDestroySemaphore(_device, _frames[i]._swapchainSemaphore, nullptr);

                _frames[i]._deletionQueue.flush();
            }

            for (auto &mesh : testMeshes)
            {
                destroy_buffer(mesh->meshBuffers.indexBuffer);
                destroy_buffer(mesh->meshBuffers.vertexBuffer);
            }

            for (auto &meshInfo : mMeshBufferBank.meshInfos)
            {
                destroy_buffer(meshInfo.second.meshBuffer.indexBuffer);
                destroy_buffer(meshInfo.second.meshBuffer.vertexBuffer);
            }

            metalRoughMaterial.clear_resources(_device);
            _mainDeletionQueue.flush();
            destroy_swapchain();
            vkDestroySurfaceKHR(_instance, _surface, nullptr);

            vkDestroyDevice(_device, nullptr);
            vkb::destroy_debug_utils_messenger(_instance, _debug_messenger);
            vkDestroyInstance(_instance, nullptr);
        }
    }

    void VulkanCore::draw_background(VkCommandBuffer cmd)
    {
        //> draw_multi
        ComputeEffect &effect = backgroundEffects[currentBackgroundEffect];

        // bind the background compute pipeline
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, effect.pipeline);

        // bind the descriptor set containing the draw image for the compute pipeline
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, _gradientPipelineLayout, 0, 1, &_drawImageDescriptors, 0, nullptr);

        vkCmdPushConstants(cmd, _gradientPipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(ComputePushConstants), &effect.data);
        // execute the compute pipeline dispatch. We are using 16x16 workgroup size so we need to divide by it
        vkCmdDispatch(cmd, std::ceil(_drawExtent.width / 16.0), std::ceil(_drawExtent.height / 16.0), 1);
        //< draw_multi
    }

    void VulkanCore::draw_geometry(VkCommandBuffer cmd, const EngineContext &context)
    {
        // begin a render pass  connected to our draw image
        VkRenderingAttachmentInfo colorAttachment = create::attachment_info(_drawImage.imageView, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        VkRenderingAttachmentInfo depthAttachment = create::depth_attachment_info(_depthImage.imageView, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

        VkRenderingInfo renderInfo = create::rendering_info(_drawExtent, &colorAttachment, &depthAttachment);
        vkCmdBeginRendering(cmd, &renderInfo);

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _meshPipeline);

        {
            // set dynamic viewport and scissor
            VkViewport viewport = {};
            viewport.x = 0;
            viewport.y = 0;
            viewport.width = _drawExtent.width;
            viewport.height = _drawExtent.height;
            viewport.minDepth = 0.f;
            viewport.maxDepth = 1.f;

            vkCmdSetViewport(cmd, 0, 1, &viewport);

            VkRect2D scissor = {};
            scissor.offset.x = 0;
            scissor.offset.y = 0;
            scissor.extent.width = _drawExtent.width;
            scissor.extent.height = _drawExtent.height;

            vkCmdSetScissor(cmd, 0, 1, &scissor);
        }

        // TODO

        // allocate a new uniform buffer for the scene data
        AllocatedBuffer gpuSceneDataBuffer = create_buffer(sizeof(GPUSceneData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

        // add it to the deletion queue of this frame so it gets deleted once its been used
        get_current_frame()._deletionQueue.push_function([=, this]()
                                                         { destroy_buffer(gpuSceneDataBuffer); });

        // write the buffer
        GPUSceneData *sceneUniformData = (GPUSceneData *)gpuSceneDataBuffer.allocation->GetMappedData();
        *sceneUniformData = sceneData;

        // create a descriptor set that binds that buffer and update it
        VkDescriptorSet globalDescriptor = get_current_frame()._frameDescriptors.allocate(_device, _gpuSceneDataDescriptorLayout);

        DescriptorWriter writer;
        writer.write_buffer(0, gpuSceneDataBuffer.buffer, sizeof(GPUSceneData), 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
        writer.update_set(_device, globalDescriptor);

        for (const RenderObject &draw : mainDrawContext.OpaqueSurfaces)
        {

            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, draw.material->pipeline->pipeline);
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, draw.material->pipeline->layout, 0, 1, &globalDescriptor, 0, nullptr);
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, draw.material->pipeline->layout, 1, 1, &draw.material->materialSet, 0, nullptr);

            vkCmdBindIndexBuffer(cmd, draw.indexBuffer, 0, VK_INDEX_TYPE_UINT32);

            GPUDrawPushConstants pushConstants;
            pushConstants.vertexBuffer = draw.vertexBufferAddress;
            pushConstants.worldMatrix = draw.transform;
            vkCmdPushConstants(cmd, draw.material->pipeline->layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPUDrawPushConstants), &pushConstants);

            vkCmdDrawIndexed(cmd, draw.indexCount, 1, draw.firstIndex, 0, 0);
        }

        {
            // GPUDrawPushConstants push_constants;

            // const auto &cInfo = context.cameraInfo[0];

            // Vec3f eye = Vec3f(cInfo.transform.col(3)[0], cInfo.transform.col(3)[1], cInfo.transform.col(3)[2]);
            // Vec3f center = eye + Vec3f(cInfo.forward.x(), cInfo.forward.y(), cInfo.forward.z());
            // Vec3f up = Vec3f(cInfo.up.x(), cInfo.up.y(), cInfo.up.z());
            // auto view = math::LookAt(eye, center, up);
            // // notice here far/near is reverted
            // auto projection = math::PerspectiveVK(cInfo.fov_radian, (float)_drawExtent.width / (float)_drawExtent.height, cInfo.near, cInfo.far);
            // // projection.col(1).y() *= -1.f;
            // push_constants.worldMatrix = projection * view;
            // // INFO_LOG("x:[{}] y:[{}] z:[{}]",eye.x(),eye.y(),eye.z());

            // push_constants.vertexBuffer = testMeshes[0]->meshBuffers.vertexBufferAddress;

            // vkCmdPushConstants(cmd, _meshPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPUDrawPushConstants), &push_constants);
            // vkCmdBindIndexBuffer(cmd, testMeshes[0]->meshBuffers.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

            // vkCmdDrawIndexed(cmd, testMeshes[0]->surfaces[0].count, 1, testMeshes[0]->surfaces[0].startIndex, 0, 0);
            // vkCmdDraw(cmd, 3, 1, 0, 0);
        }

        vkCmdEndRendering(cmd);
    }

    void VulkanCore::test_update_scene(const EngineContext &context)
    {
        // mainDrawContext.OpaqueSurfaces.clear();

        // for (auto &m : loadedNodes)
        // {
        //     m.second->Draw(glm::mat4{1.f}, mainDrawContext);
        // }

        // for (int x = -3; x < 3; x++)
        // {

        //     glm::mat4 scale = glm::scale(glm::vec3{0.2});
        //     glm::mat4 translation = glm::translate(glm::vec3{x, 1, 0});

        //     loadedNodes["Cube"]->Draw(translation * scale, mainDrawContext);
        // }

        const auto &cInfo = context.cameraInfo[0];

        Vec3f eye = Vec3f(cInfo.transform.col(3)[0], cInfo.transform.col(3)[1], cInfo.transform.col(3)[2]);
        Vec3f center = eye + Vec3f(cInfo.forward.x(), cInfo.forward.y(), cInfo.forward.z());
        Vec3f up = Vec3f(cInfo.up.x(), cInfo.up.y(), cInfo.up.z());
        auto view = math::LookAt(eye, center, up);
        auto projection = math::PerspectiveVK(cInfo.fov_radian, (float)_drawExtent.width / (float)_drawExtent.height, cInfo.near, cInfo.far);

        sceneData.view = view;
        // camera projection
        sceneData.proj = projection;

        sceneData.viewproj = sceneData.proj * sceneData.view;
    }

    void VulkanCore::draw(u32 width, u32 height, const EngineContext &context)
    {
        // temp
        test_update_scene(context);

        //> frame_clear
        // wait until the gpu has finished rendering the last frame. Timeout of 1 second
        VK_CHECK(vkWaitForFences(_device, 1, &get_current_frame()._renderFence, true, 1000000000));

        get_current_frame()._deletionQueue.flush();
        get_current_frame()._frameDescriptors.clear_pools(_device);
        //< frame_clear

        // request image from the swapchain
        uint32_t swapchainImageIndex;

        VkResult e = vkAcquireNextImageKHR(_device, _swapchain, 1000000000, get_current_frame()._swapchainSemaphore, nullptr, &swapchainImageIndex);
        if (e == VK_ERROR_OUT_OF_DATE_KHR)
        {
            // todo
            resize_requested = true;
            return;
        }

        _drawExtent.height = std::min(_swapchainExtent.height, _drawImage.imageExtent.height) * renderScale;
        _drawExtent.width = std::min(_swapchainExtent.width, _drawImage.imageExtent.width) * renderScale;

        VK_CHECK(vkResetFences(_device, 1, &get_current_frame()._renderFence));

        // now that we are sure that the commands finished executing, we can safely reset the command buffer to begin recording again.
        VK_CHECK(vkResetCommandBuffer(get_current_frame()._mainCommandBuffer, 0));

        // naming it cmd for shorter writing
        VkCommandBuffer cmd = get_current_frame()._mainCommandBuffer;

        // begin the command buffer recording. We will use this command buffer exactly once, so we want to let vulkan know that
        VkCommandBufferBeginInfo cmdBeginInfo = create::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

        VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

        // transition our main draw image into general layout so we can write into it
        // we will overwrite it all so we dont care about what was the older layout
        utils::transition_image(cmd, _drawImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

        draw_background(cmd);

        utils::transition_image(cmd, _drawImage.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        utils::transition_image(cmd, _depthImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

        draw_geometry(cmd, context);

        // transition the draw image and the swapchain image into their correct transfer layouts
        utils::transition_image(cmd, _drawImage.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
        utils::transition_image(cmd, _swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        //< draw_first
        //> imgui_draw
        // execute a copy from the draw image into the swapchain
        utils::copy_image_to_image(cmd, _drawImage.image, _swapchainImages[swapchainImageIndex], _drawExtent, _swapchainExtent);

        // set swapchain image layout to Attachment Optimal so we can draw it
        utils::transition_image(cmd, _swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

        {
            // draw imgui into the swapchain image
            ui::IMGUI_VULKAN_GLFW::VkDrawImgui(this, cmd, _swapchainImageViews[swapchainImageIndex]);
        }

        // set swapchain image layout to Present so we can draw it
        utils::transition_image(cmd, _swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

        // finalize the command buffer (we can no longer add commands, but it can now be executed)
        VK_CHECK(vkEndCommandBuffer(cmd));
        //< imgui_draw

        // prepare the submission to the queue.
        // we want to wait on the _presentSemaphore, as that semaphore is signaled when the swapchain is ready
        // we will signal the _renderSemaphore, to signal that rendering has finished

        VkCommandBufferSubmitInfo cmdinfo = create::command_buffer_submit_info(cmd);

        VkSemaphoreSubmitInfo waitInfo = create::semaphore_submit_info(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, get_current_frame()._swapchainSemaphore);
        VkSemaphoreSubmitInfo signalInfo = create::semaphore_submit_info(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, get_current_frame()._renderSemaphore);

        VkSubmitInfo2 submit = create::submit_info(&cmdinfo, &signalInfo, &waitInfo);

        // submit command buffer to the queue and execute it.
        //  _renderFence will now block until the graphic commands finish execution
        VK_CHECK(vkQueueSubmit2(_graphicsQueue, 1, &submit, get_current_frame()._renderFence));

        // prepare present
        //  this will put the image we just rendered to into the visible window.
        //  we want to wait on the _renderSemaphore for that,
        //  as its necessary that drawing commands have finished before the image is displayed to the user
        VkPresentInfoKHR presentInfo = create::present_info();

        presentInfo.pSwapchains = &_swapchain;
        presentInfo.swapchainCount = 1;

        presentInfo.pWaitSemaphores = &get_current_frame()._renderSemaphore;
        presentInfo.waitSemaphoreCount = 1;

        presentInfo.pImageIndices = &swapchainImageIndex;

        // TODO check for optimization
        VkResult presentResult = vkQueuePresentKHR(_graphicsQueue, &presentInfo);

        // if (presentResult == VK_ERROR_OUT_OF_DATE_KHR)
        // {
        //     resize_requested = true;
        //     return;
        // }

        // increase the number of frames drawn
        _frameNumber++;
    }

    void VulkanCore::init_vulkan()
    {
        vkb::InstanceBuilder builder;

        // make the vulkan instance, with basic debug features
        auto inst_ret = builder.set_app_name("Example Vulkan Application")
                            .request_validation_layers(bUseValidationLayers)
                            .use_default_debug_messenger()
                            .require_api_version(1, 3, 0)
                            .build();

        vkb::Instance vkb_inst = inst_ret.value();

        // grab the instance
        _instance = vkb_inst.instance;
        _debug_messenger = vkb_inst.debug_messenger;

#if defined(USE_GLFW)
        VK_CHECK(glfwCreateWindowSurface(_instance, (GLFWwindow *)(_windowPtr), NULL, &_surface));
#endif

        // vulkan 1.3 features
        VkPhysicalDeviceVulkan13Features features13{};
        features13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
        features13.dynamicRendering = true;
        features13.synchronization2 = true;

        // vulkan 1.2 features
        VkPhysicalDeviceVulkan12Features features12{};
        features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
        features12.bufferDeviceAddress = true;
        features12.descriptorIndexing = true;

        vkb::PhysicalDeviceSelector selector{vkb_inst};
        vkb::PhysicalDevice physicalDevice = selector
                                                 .set_minimum_version(1, 3)
                                                 .set_required_features_13(features13)
                                                 .set_required_features_12(features12)
                                                 .set_surface(_surface)
                                                 .select()
                                                 .value();

        // create the final vulkan device
        vkb::DeviceBuilder deviceBuilder{physicalDevice};
        vkb::Device vkbDevice = deviceBuilder.build().value();

        // Get the VkDevice handle used in the rest of a vulkan application
        _device = vkbDevice.device;
        _chosenGPU = physicalDevice.physical_device;

        // use vkbootstrap to get a Graphics queue
        _graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
        _graphicsQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

        //> vma_init
        // initialize the memory allocator
        VmaAllocatorCreateInfo allocatorInfo = {};
        allocatorInfo.physicalDevice = _chosenGPU;
        allocatorInfo.device = _device;
        allocatorInfo.instance = _instance;
        allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
        vmaCreateAllocator(&allocatorInfo, &_allocator);

        _mainDeletionQueue.push_function([&]()
                                         { vmaDestroyAllocator(_allocator); });
        //< vma_init
    }

    void VulkanCore::create_swapchain(uint32_t width, uint32_t height)
    {
        vkb::SwapchainBuilder swapchainBuilder{_chosenGPU, _device, _surface};

        _swapchainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;

        vkb::Swapchain vkbSwapchain = swapchainBuilder
                                          //.use_default_format_selection()
                                          .set_desired_format(VkSurfaceFormatKHR{.format = _swapchainImageFormat, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR})
                                          // use vsync present mode
                                          .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
                                          .set_desired_extent(width, height)
                                          .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
                                          .build()
                                          .value();

        _swapchainExtent = vkbSwapchain.extent;
        // store swapchain and its related images
        _swapchain = vkbSwapchain.swapchain;
        _swapchainImages = vkbSwapchain.get_images().value();
        _swapchainImageViews = vkbSwapchain.get_image_views().value();
    }

    void VulkanCore::destroy_swapchain()
    {
        vkDestroySwapchainKHR(_device, _swapchain, nullptr);
        for (u32 i = 0; i < _swapchainImageViews.size(); i++)
        {
            vkDestroyImageView(_device, _swapchainImageViews[i], nullptr);
        }
    }

    void VulkanCore::resize_swapchain(u32 width, u32 height)
    {
        vkDeviceWaitIdle(_device);

        destroy_swapchain();

        _windowExtent.width = width;
        _windowExtent.height = height;

        create_swapchain(_windowExtent.width, _windowExtent.height);

        resize_requested = false;
    }

    void VulkanCore::init_swapchain()
    {
        create_swapchain(_windowExtent.width, _windowExtent.height);

        VkExtent3D drawImageExtent = {
            _windowExtent.width,
            _windowExtent.height,
            1};

        // hardcoding the draw format to 32 bit float
        _drawImage.imageFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
        _drawImage.imageExtent = drawImageExtent;

        VkImageUsageFlags drawImageUsages{};
        drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        // drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        drawImageUsages |= VK_IMAGE_USAGE_STORAGE_BIT;
        drawImageUsages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        VkImageCreateInfo rimg_info = create::image_create_info(_drawImage.imageFormat, drawImageUsages, drawImageExtent);

        // for the draw image, we want to allocate it from gpu local memory
        VmaAllocationCreateInfo rimg_allocinfo = {};
        rimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        rimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        // rimg_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        //  allocate and create the image
        vmaCreateImage(_allocator, &rimg_info, &rimg_allocinfo, &_drawImage.image, &_drawImage.allocation, nullptr);

        // build a image-view for the draw image to use for rendering
        VkImageViewCreateInfo rview_info = create::imageview_create_info(_drawImage.imageFormat, _drawImage.image, VK_IMAGE_ASPECT_COLOR_BIT);

        VK_CHECK(vkCreateImageView(_device, &rview_info, nullptr, &_drawImage.imageView));

        //> depthimg
        _depthImage.imageFormat = VK_FORMAT_D32_SFLOAT;
        _depthImage.imageExtent = drawImageExtent;

        VkImageUsageFlags depthImageUsages{};
        depthImageUsages |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

        VkImageCreateInfo dimg_info = create::image_create_info(_depthImage.imageFormat, depthImageUsages, drawImageExtent);

        // allocate and create the image
        VK_CHECK(vmaCreateImage(_allocator, &dimg_info, &rimg_allocinfo, &_depthImage.image, &_depthImage.allocation, nullptr));

        // build a image-view for the draw image to use for rendering
        VkImageViewCreateInfo dview_info = create::imageview_create_info(_depthImage.imageFormat, _depthImage.image, VK_IMAGE_ASPECT_DEPTH_BIT);

        VK_CHECK(vkCreateImageView(_device, &dview_info, nullptr, &_depthImage.imageView));
        //< depthimg

        // add to deletion queues
        _mainDeletionQueue.push_function([this]()
                                         {
                                             destroy_image(_depthImage);
                                             destroy_image(_drawImage);
                                             // vkDestroyImageView(_device, _drawImage.imageView, nullptr);
                                             // vmaDestroyImage(_allocator, _drawImage.image, _drawImage.allocation);
                                             // vkDestroyImageView(_device, _depthImage.imageView, nullptr);
                                             // vmaDestroyImage(_allocator, _depthImage.image, _depthImage.allocation);
                                         });
        //< init_swap
    }

    void VulkanCore::init_commands()
    {
        // create a command pool for commands submitted to the graphics queue.
        // we also want the pool to allow for resetting of individual command buffers
        VkCommandPoolCreateInfo commandPoolInfo = create::command_pool_create_info(_graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

        for (int i = 0; i < FRAME_OVERLAP; i++)
        {

            VK_CHECK(vkCreateCommandPool(_device, &commandPoolInfo, nullptr, &_frames[i]._commandPool));

            // allocate the default command buffer that we will use for rendering
            VkCommandBufferAllocateInfo cmdAllocInfo = create::command_buffer_allocate_info(_frames[i]._commandPool, 1);

            VK_CHECK(vkAllocateCommandBuffers(_device, &cmdAllocInfo, &_frames[i]._mainCommandBuffer));
        }

        //> imm_cmd
        VK_CHECK(vkCreateCommandPool(_device, &commandPoolInfo, nullptr, &_immCommandPool));

        // allocate the command buffer for immediate submits
        VkCommandBufferAllocateInfo cmdAllocInfo = create::command_buffer_allocate_info(_immCommandPool, 1);

        VK_CHECK(vkAllocateCommandBuffers(_device, &cmdAllocInfo, &_immCommandBuffer));

        _mainDeletionQueue.push_function([this]()
                                         { vkDestroyCommandPool(_device, _immCommandPool, nullptr); });
        //< imm_cmd
    }

    void VulkanCore::init_background_pipelines()
    {
        VkPipelineLayoutCreateInfo computeLayout{};
        computeLayout.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        computeLayout.pNext = nullptr;
        computeLayout.pSetLayouts = &_drawImageDescriptorLayout;
        computeLayout.setLayoutCount = 1;

        VkPushConstantRange pushConstant{};
        pushConstant.offset = 0;
        pushConstant.size = sizeof(ComputePushConstants);
        pushConstant.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

        computeLayout.pPushConstantRanges = &pushConstant;
        computeLayout.pushConstantRangeCount = 1;

        VK_CHECK(vkCreatePipelineLayout(_device, &computeLayout, nullptr, &_gradientPipelineLayout));
        //> comp_pipeline_multi
        VkShaderModule gradientShader;
        std::string gradientShaderPath = config::shader_folder_path + "gradient_color.comp.spv";
        if (!utils::load_shader_module(gradientShaderPath.data(), _device, &gradientShader))
        {
            fmt::print("Error when building the compute shader \n");
        }

        VkShaderModule skyShader;
        std::string skyShaderPath = config::shader_folder_path + "sky.comp.spv";
        if (!utils::load_shader_module(skyShaderPath.data(), _device, &skyShader))
        {
            fmt::print("Error when building the compute shader \n");
        }

        VkPipelineShaderStageCreateInfo stageinfo{};
        stageinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stageinfo.pNext = nullptr;
        stageinfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        stageinfo.module = gradientShader;
        stageinfo.pName = "main";

        VkComputePipelineCreateInfo computePipelineCreateInfo{};
        computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        computePipelineCreateInfo.pNext = nullptr;
        computePipelineCreateInfo.layout = _gradientPipelineLayout;
        computePipelineCreateInfo.stage = stageinfo;

        ComputeEffect gradient;
        gradient.layout = _gradientPipelineLayout;
        gradient.name = "gradient";
        gradient.data = {};

        // default colors
        gradient.data.data1 = Vec4f(1, 0, 0, 1);
        gradient.data.data2 = Vec4f(0, 0, 1, 1);

        VK_CHECK(vkCreateComputePipelines(_device, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, &gradient.pipeline));

        // change the shader module only to create the sky shader
        computePipelineCreateInfo.stage.module = skyShader;

        ComputeEffect sky;
        sky.layout = _gradientPipelineLayout;
        sky.name = "sky";
        sky.data = {};
        // default sky parameters
        sky.data.data1 = Vec4f(0.1, 0.2, 0.4, 0.97);

        VK_CHECK(vkCreateComputePipelines(_device, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, &sky.pipeline));

        // add the 2 background effects into the array
        backgroundEffects.push_back(gradient);
        backgroundEffects.push_back(sky);

        // destroy structures properly
        vkDestroyShaderModule(_device, gradientShader, nullptr);
        vkDestroyShaderModule(_device, skyShader, nullptr);
        _mainDeletionQueue.push_function([=, this]()
                                         {
	    vkDestroyPipelineLayout(_device, _gradientPipelineLayout, nullptr);
	    vkDestroyPipeline(_device, sky.pipeline, nullptr);
	    vkDestroyPipeline(_device, gradient.pipeline, nullptr); });
    }

    void VulkanCore::init_sync_structures()
    {
        // create syncronization structures
        // one fence to control when the gpu has finished rendering the frame,
        // and 2 semaphores to syncronize rendering with swapchain
        // we want the fence to start signalled so we can wait on it on the first frame
        VkFenceCreateInfo fenceCreateInfo = create::fence_create_info(VK_FENCE_CREATE_SIGNALED_BIT);
        VkSemaphoreCreateInfo semaphoreCreateInfo = create::semaphore_create_info();

        for (int i = 0; i < FRAME_OVERLAP; i++)
        {
            VK_CHECK(vkCreateFence(_device, &fenceCreateInfo, nullptr, &_frames[i]._renderFence));

            VK_CHECK(vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &_frames[i]._swapchainSemaphore));
            VK_CHECK(vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &_frames[i]._renderSemaphore));
        }

        //> imm_fence
        VK_CHECK(vkCreateFence(_device, &fenceCreateInfo, nullptr, &_immFence));
        _mainDeletionQueue.push_function([this]()
                                         { vkDestroyFence(_device, _immFence, nullptr); });
        //< imm_fence
    }

    void VulkanCore::immediate_submit(std::function<void(VkCommandBuffer cmd)> &&function)
    {
        VK_CHECK(vkResetFences(_device, 1, &_immFence));
        VK_CHECK(vkResetCommandBuffer(_immCommandBuffer, 0));

        VkCommandBuffer cmd = _immCommandBuffer;

        VkCommandBufferBeginInfo cmdBeginInfo = create::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

        VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

        function(cmd);

        VK_CHECK(vkEndCommandBuffer(cmd));

        VkCommandBufferSubmitInfo cmdinfo = create::command_buffer_submit_info(cmd);
        VkSubmitInfo2 submit = create::submit_info(&cmdinfo, nullptr, nullptr);

        // submit command buffer to the queue and execute it.
        //  _renderFence will now block until the graphic commands finish execution
        VK_CHECK(vkQueueSubmit2(_graphicsQueue, 1, &submit, _immFence));

        VK_CHECK(vkWaitForFences(_device, 1, &_immFence, true, 9999999999));
    }

    void VulkanCore::init_pipelines()
    {
        init_background_pipelines();

        init_triangle_pipeline();
        init_mesh_pipeline();

        metalRoughMaterial.build_pipelines(this);
    }

    void VulkanCore::init_triangle_pipeline()
    {
        //> triangle_shaders
        VkShaderModule triangleFragShader;
        std::string triangleFrag = config::shader_folder_path + "colored_triangle.frag.spv";
        if (!utils::load_shader_module(triangleFrag.data(), _device, &triangleFragShader))
        {
            fmt::print("Error when building the triangle fragment shader module");
        }
        else
        {
            fmt::print("Triangle fragment shader succesfully loaded");
        }

        VkShaderModule triangleVertexShader;
        std::string triangleVert = config::shader_folder_path + "colored_triangle.vert.spv";
        if (!utils::load_shader_module(triangleVert.data(), _device, &triangleVertexShader))
        {
            fmt::print("Error when building the triangle vertex shader module");
        }
        else
        {
            fmt::print("Triangle vertex shader succesfully loaded");
        }

        // build the pipeline layout that controls the inputs/outputs of the shader
        // we are not using descriptor sets or other systems yet, so no need to use anything other than empty default
        VkPipelineLayoutCreateInfo pipeline_layout_info = create::pipeline_layout_create_info();
        VK_CHECK(vkCreatePipelineLayout(_device, &pipeline_layout_info, nullptr, &_trianglePipelineLayout));
        //< triangle_shaders

        PipelineBuilder pipelineBuilder;

        // use the triangle layout we created
        pipelineBuilder._pipelineLayout = _trianglePipelineLayout;
        // connecting the vertex and pixel shaders to the pipeline
        pipelineBuilder.set_shaders(triangleVertexShader, triangleFragShader);
        // it will draw triangles
        pipelineBuilder.set_input_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        // filled triangles
        pipelineBuilder.set_polygon_mode(VK_POLYGON_MODE_FILL);
        // no backface culling
        pipelineBuilder.set_cull_mode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);
        // no multisampling
        pipelineBuilder.set_multisampling_none();
        // no blending
        pipelineBuilder.disable_blending();
        // no depth testing
        pipelineBuilder.disable_depthtest();

        // connect the image format we will draw into, from draw image
        pipelineBuilder.set_color_attachment_format(_drawImage.imageFormat);
        pipelineBuilder.set_depth_format(_depthImage.imageFormat);

        // finally build the pipeline
        _trianglePipeline = pipelineBuilder.build_pipeline(_device);

        // clean structures
        vkDestroyShaderModule(_device, triangleFragShader, nullptr);
        vkDestroyShaderModule(_device, triangleVertexShader, nullptr);

        _mainDeletionQueue.push_function([&]()
                                         {
		vkDestroyPipelineLayout(_device, _trianglePipelineLayout, nullptr);
		vkDestroyPipeline(_device, _trianglePipeline, nullptr); });
    }

    void VulkanCore::init_mesh_pipeline()
    {
        //> rectangle_shaders
        VkShaderModule triangleFragShader;
        std::string fragShaderPath = config::shader_folder_path + "colored_triangle.frag.spv";
        if (!utils::load_shader_module(fragShaderPath.data(), _device, &triangleFragShader))
        {
            fmt::print("Error when building the triangle fragment shader module");
        }
        else
        {
            fmt::print("Triangle fragment shader succesfully loaded");
        }

        VkShaderModule triangleVertexShader;
        std::string vertShaderPath = config::shader_folder_path + "colored_triangle_mesh.vert.spv";
        if (!utils::load_shader_module(vertShaderPath.data(), _device, &triangleVertexShader))
        {
            fmt::print("Error when building the triangle vertex shader module");
        }
        else
        {
            fmt::print("Triangle vertex shader succesfully loaded");
        }

        VkPushConstantRange bufferRange{};
        bufferRange.offset = 0;
        bufferRange.size = sizeof(GPUDrawPushConstants);
        bufferRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        VkPipelineLayoutCreateInfo pipeline_layout_info = create::pipeline_layout_create_info();
        pipeline_layout_info.pPushConstantRanges = &bufferRange;
        pipeline_layout_info.pushConstantRangeCount = 1;

        VK_CHECK(vkCreatePipelineLayout(_device, &pipeline_layout_info, nullptr, &_meshPipelineLayout));

        //< rectangle_shaders

        // exactly same as above but with the depth testing set
        PipelineBuilder pipelineBuilder;

        // use the triangle layout we created
        pipelineBuilder._pipelineLayout = _meshPipelineLayout;
        // connecting the vertex and pixel shaders to the pipeline
        pipelineBuilder.set_shaders(triangleVertexShader, triangleFragShader);
        // it will draw triangles
        pipelineBuilder.set_input_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        // filled triangles
        pipelineBuilder.set_polygon_mode(VK_POLYGON_MODE_FILL);
        // no backface culling
        pipelineBuilder.set_cull_mode(VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE);
        // no multisampling
        pipelineBuilder.set_multisampling_none();
        // no blending
        pipelineBuilder.disable_blending();
        // pipelineBuilder.enable_blending_additive();

        pipelineBuilder.disable_depthtest();
        // pipelineBuilder.enable_depthtest(true, VK_COMPARE_OP_LESS_OR_EQUAL);

        // connect the image format we will draw into, from draw image
        pipelineBuilder.set_color_attachment_format(_drawImage.imageFormat);
        pipelineBuilder.set_depth_format(_depthImage.imageFormat);

        // finally build the pipeline
        _meshPipeline = pipelineBuilder.build_pipeline(_device);

        // clean structures
        vkDestroyShaderModule(_device, triangleFragShader, nullptr);
        vkDestroyShaderModule(_device, triangleVertexShader, nullptr);

        _mainDeletionQueue.push_function([&]()
                                         {
		vkDestroyPipelineLayout(_device, _meshPipelineLayout, nullptr);
		vkDestroyPipeline(_device, _meshPipeline, nullptr); });
    }

    void VulkanCore::init_descriptors()
    {
        //> init_desc_1
        // create a descriptor pool that will hold 10 sets with 1 image each
        std::vector<DescriptorAllocatorGrowable::PoolSizeRatio> sizes =
            {
                {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1},
                {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1}};

        globalDescriptorAllocator.init(_device, 10, sizes);

        // make the descriptor set layout for our compute draw
        {
            DescriptorLayoutBuilder builder;
            builder.add_binding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
            _drawImageDescriptorLayout = builder.build(_device, VK_SHADER_STAGE_COMPUTE_BIT);
        }
        {
            DescriptorLayoutBuilder builder;
            builder.add_binding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
            _singleImageDescriptorLayout = builder.build(_device, VK_SHADER_STAGE_FRAGMENT_BIT);
        }
        {
            DescriptorLayoutBuilder builder;
            builder.add_binding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
            _gpuSceneDataDescriptorLayout = builder.build(_device, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
        }
        // allocate a descriptor set for our draw image
        _drawImageDescriptors = globalDescriptorAllocator.allocate(_device, _drawImageDescriptorLayout);

        {
            DescriptorWriter writer;
            writer.write_image(0, _drawImage.imageView, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_GENERAL, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);

            writer.update_set(_device, _drawImageDescriptors);
        }

        // make sure both the descriptor allocator and the new layout get cleaned up properly
        _mainDeletionQueue.push_function([&]()
                                         {
		globalDescriptorAllocator.destroy_pools(_device);

		vkDestroyDescriptorSetLayout(_device, _drawImageDescriptorLayout, nullptr);
		vkDestroyDescriptorSetLayout(_device, _singleImageDescriptorLayout, nullptr);
		vkDestroyDescriptorSetLayout(_device, _gpuSceneDataDescriptorLayout, nullptr); });

        //> frame_desc
        for (int i = 0; i < FRAME_OVERLAP; i++)
        {
            // create a descriptor pool
            std::vector<DescriptorAllocatorGrowable::PoolSizeRatio> frame_sizes = {
                {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 3},
                {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3},
                {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3},
                {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4},
            };

            _frames[i]._frameDescriptors = DescriptorAllocatorGrowable{};
            _frames[i]._frameDescriptors.init(_device, 1000, frame_sizes);

            _mainDeletionQueue.push_function([&, i]()
                                             { _frames[i]._frameDescriptors.destroy_pools(_device); });
        }
        //< frame_desc
    }

    AllocatedBuffer VulkanCore::create_buffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage)
    {
        // allocate buffer
        VkBufferCreateInfo bufferInfo = {.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        bufferInfo.pNext = nullptr;
        bufferInfo.size = allocSize;

        bufferInfo.usage = usage;

        VmaAllocationCreateInfo vmaallocInfo = {};
        vmaallocInfo.usage = memoryUsage;
        vmaallocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
        AllocatedBuffer newBuffer;

        // allocate the buffer
        VK_CHECK(vmaCreateBuffer(_allocator, &bufferInfo, &vmaallocInfo, &newBuffer.buffer, &newBuffer.allocation,
                                 &newBuffer.info));

        return newBuffer;
    }

    AllocatedImage VulkanCore::create_image(VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped)
    {
        AllocatedImage newImage;
        newImage.imageFormat = format;
        newImage.imageExtent = size;

        VkImageCreateInfo img_info = create::image_create_info(format, usage, size);
        if (mipmapped)
        {
            img_info.mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(size.width, size.height)))) + 1;
        }

        // always allocate images on dedicated GPU memory
        VmaAllocationCreateInfo allocinfo = {};
        allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        // allocate and create the image
        VK_CHECK(vmaCreateImage(_allocator, &img_info, &allocinfo, &newImage.image, &newImage.allocation, nullptr));

        // if the format is a depth format, we will need to have it use the correct
        // aspect flag
        VkImageAspectFlags aspectFlag = VK_IMAGE_ASPECT_COLOR_BIT;
        if (format == VK_FORMAT_D32_SFLOAT)
        {
            aspectFlag = VK_IMAGE_ASPECT_DEPTH_BIT;
        }

        // build a image-view for the image
        VkImageViewCreateInfo view_info = create::imageview_create_info(format, newImage.image, aspectFlag);
        view_info.subresourceRange.levelCount = img_info.mipLevels;

        VK_CHECK(vkCreateImageView(_device, &view_info, nullptr, &newImage.imageView));

        return newImage;
    }

    void VulkanCore::destroy_image(const AllocatedImage &img)
    {
        vkDestroyImageView(_device, img.imageView, nullptr);
        vmaDestroyImage(_allocator, img.image, img.allocation);
    }

    AllocatedImage VulkanCore::create_image(void *data, VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped)
    {
        size_t data_size = size.depth * size.width * size.height * 4;
        AllocatedBuffer uploadbuffer = create_buffer(data_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

        memcpy(uploadbuffer.info.pMappedData, data, data_size);

        AllocatedImage new_image = create_image(size, format, usage | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, mipmapped);

        immediate_submit([&](VkCommandBuffer cmd)
                         {
		utils::transition_image(cmd, new_image.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		VkBufferImageCopy copyRegion = {};
		copyRegion.bufferOffset = 0;
		copyRegion.bufferRowLength = 0;
		copyRegion.bufferImageHeight = 0;

		copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copyRegion.imageSubresource.mipLevel = 0;
		copyRegion.imageSubresource.baseArrayLayer = 0;
		copyRegion.imageSubresource.layerCount = 1;
		copyRegion.imageExtent = size;

		// copy the buffer into the image
		vkCmdCopyBufferToImage(cmd, uploadbuffer.buffer, new_image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
			&copyRegion);

		utils::transition_image(cmd, new_image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL); });

        destroy_buffer(uploadbuffer);

        return new_image;
    }

    void VulkanCore::destroy_buffer(const AllocatedBuffer &buffer)
    {
        vmaDestroyBuffer(_allocator, buffer.buffer, buffer.allocation);
    }

    GPUMeshBufferHandle VulkanCore::createGPUMesh(std::span<uint32_t> indices, std::span<asset::Vertex> vertices)
    {
        GPUMeshBuffers meshBuffers = uploadMesh(indices, vertices);
        return GPUMeshBufferHandle();
        // return mGPUMeshBuffersMap.AddResource(meshBuffers);
    }

    GPUMeshBuffers VulkanCore::uploadMesh(std::span<uint32_t> indices, std::span<asset::Vertex> vertices)
    {
        //> mesh_create_1
        const size_t vertexBufferSize = vertices.size() * sizeof(asset::VkVertex);
        const size_t indexBufferSize = indices.size() * sizeof(uint32_t);

        GPUMeshBuffers newSurface;

        // create vertex buffer
        newSurface.vertexBuffer = create_buffer(vertexBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                                                VMA_MEMORY_USAGE_GPU_ONLY);

        // find the adress of the vertex buffer
        VkBufferDeviceAddressInfo deviceAdressInfo{.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, .buffer = newSurface.vertexBuffer.buffer};
        newSurface.vertexBufferAddress = vkGetBufferDeviceAddress(_device, &deviceAdressInfo);

        // create index buffer
        newSurface.indexBuffer = create_buffer(indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                               VMA_MEMORY_USAGE_GPU_ONLY);

        //< mesh_create_1
        //
        //> mesh_create_2
        AllocatedBuffer staging = create_buffer(vertexBufferSize + indexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

        void *data = staging.allocation->GetMappedData();

        // copy vertex buffer
        memcpy(data, vertices.data(), vertexBufferSize);
        // copy index buffer
        memcpy((char *)data + vertexBufferSize, indices.data(), indexBufferSize);

        immediate_submit([&](VkCommandBuffer cmd)
                         {
		VkBufferCopy vertexCopy{
            0 };
		vertexCopy.dstOffset = 0;
		vertexCopy.srcOffset = 0;
		vertexCopy.size = vertexBufferSize;

		vkCmdCopyBuffer(cmd, staging.buffer, newSurface.vertexBuffer.buffer, 1, &vertexCopy);

		VkBufferCopy indexCopy{ 0 };
		indexCopy.dstOffset = 0;
		indexCopy.srcOffset = vertexBufferSize;
		indexCopy.size = indexBufferSize;

		vkCmdCopyBuffer(cmd, staging.buffer, newSurface.indexBuffer.buffer, 1, &indexCopy); });

        destroy_buffer(staging);

        return newSurface;
        //< mesh_create_2
    }

    void GLTFMetallic_Roughness::build_pipelines(VulkanCore *engine)
    {
        VkShaderModule meshFragShader;
        std::string fragPath = config::shader_folder_path + "mesh.frag.spv";
        if (!utils::load_shader_module(fragPath.data(), engine->_device, &meshFragShader))
        {
            INFO_PRINT("Error when building the triangle fragment shader module");
        }

        VkShaderModule meshVertexShader;
        std::string vertPath = config::shader_folder_path + "mesh.vert.spv";
        if (!utils::load_shader_module(vertPath.data(), engine->_device, &meshVertexShader))
        {
            INFO_PRINT("Error when building the triangle vertex shader module");
        }

        VkPushConstantRange matrixRange{};
        matrixRange.offset = 0;
        matrixRange.size = sizeof(GPUDrawPushConstants);
        matrixRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        DescriptorLayoutBuilder layoutBuilder;
        layoutBuilder.add_binding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
        layoutBuilder.add_binding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
        layoutBuilder.add_binding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

        materialLayout = layoutBuilder.build(engine->_device, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);

        VkDescriptorSetLayout layouts[] = {engine->_gpuSceneDataDescriptorLayout,
                                           materialLayout};

        VkPipelineLayoutCreateInfo mesh_layout_info = create::pipeline_layout_create_info();
        mesh_layout_info.setLayoutCount = 2;
        mesh_layout_info.pSetLayouts = layouts;
        mesh_layout_info.pPushConstantRanges = &matrixRange;
        mesh_layout_info.pushConstantRangeCount = 1;

        VkPipelineLayout newLayout;
        VK_CHECK(vkCreatePipelineLayout(engine->_device, &mesh_layout_info, nullptr, &newLayout));

        opaquePipeline.layout = newLayout;
        transparentPipeline.layout = newLayout;

        // build the stage-create-info for both vertex and fragment stages. This lets
        // the pipeline know the shader modules per stage
        PipelineBuilder pipelineBuilder;

        pipelineBuilder.set_shaders(meshVertexShader, meshFragShader);

        pipelineBuilder.set_input_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

        pipelineBuilder.set_polygon_mode(VK_POLYGON_MODE_FILL);

        pipelineBuilder.set_cull_mode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);

        pipelineBuilder.set_multisampling_none();

        pipelineBuilder.disable_blending();

        pipelineBuilder.enable_depthtest(true, VK_COMPARE_OP_LESS_OR_EQUAL);

        // render format
        pipelineBuilder.set_color_attachment_format(engine->_drawImage.imageFormat);
        pipelineBuilder.set_depth_format(engine->_depthImage.imageFormat);

        // use the triangle layout we created
        pipelineBuilder._pipelineLayout = newLayout;

        // finally build the pipeline
        opaquePipeline.pipeline = pipelineBuilder.build_pipeline(engine->_device);

        // create the transparent variant
        pipelineBuilder.enable_blending_additive();

        pipelineBuilder.enable_depthtest(false, VK_COMPARE_OP_GREATER_OR_EQUAL);

        transparentPipeline.pipeline = pipelineBuilder.build_pipeline(engine->_device);

        vkDestroyShaderModule(engine->_device, meshFragShader, nullptr);
        vkDestroyShaderModule(engine->_device, meshVertexShader, nullptr);
    }

    void GLTFMetallic_Roughness::clear_resources(VkDevice device)
    {
        vkDestroyDescriptorSetLayout(device, materialLayout, nullptr);
        vkDestroyPipelineLayout(device, transparentPipeline.layout, nullptr);

        vkDestroyPipeline(device, transparentPipeline.pipeline, nullptr);
        vkDestroyPipeline(device, opaquePipeline.pipeline, nullptr);
    }

    MaterialInstance GLTFMetallic_Roughness::write_material(VkDevice device, MaterialPass pass, const MaterialResources &resources, DescriptorAllocatorGrowable &descriptorAllocator)
    {
        MaterialInstance matData;
        matData.passType = pass;
        if (pass == MaterialPass::Transparent)
        {
            matData.pipeline = &transparentPipeline;
        }
        else
        {
            matData.pipeline = &opaquePipeline;
        }

        matData.materialSet = descriptorAllocator.allocate(device, materialLayout);

        writer.clear();
        writer.write_buffer(0, resources.dataBuffer, sizeof(MaterialConstants), resources.dataBufferOffset, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
        writer.write_image(1, resources.colorImage.imageView, resources.colorSampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
        writer.write_image(2, resources.metalRoughImage.imageView, resources.metalRoughSampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

        writer.update_set(device, matData.materialSet);

        return matData;
    }
}