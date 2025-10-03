#include <GFX/Entities/Painters.h>
#include <GFX/Entities/Model.h>
#include <ImguiHandler.h>
#include <GFX/Entities/RenderScene.h>
#include <GFX/Entities/Pipeline.h>

//TODO: Remove this from here
#include <GFX/Vulkan/VulkanRenderer.h>
#include <GFX/Vulkan/Internal/Entities/VulkanPipeline.h>
#include <stb/stb_image.h>
#include <iostream>

namespace Bolt
{
    GeometryPainter::GeometryPainter(Quaint::IMemoryContext* context, const Quaint::QName& pipeline)
    : Painter(context, pipeline)
    , m_models(context)
    {
    }

    void GeometryPainter::preRender(RenderScene* scene, uint32_t stage)
    {

        //TODO:
    }
    void GeometryPainter::render(RenderScene* scene)
    {
        for(auto& model : m_models)
        {
            model->draw(scene);
            //TODO:
        }
    }
    void GeometryPainter::postRender()
    {
        //TODO:
    }

    void GeometryPainter::AddModel(Model* model)
    {
        m_models.pushBack(model);
    }


    //IMGGUI Painter
    const int ALIGNMENT = 64;


    ImguiPainter::ImguiPainter(Quaint::IMemoryContext* context, const Quaint::QName& pipeline)
    : Painter(context, pipeline)
    , m_descriptorInfo(context)
    {
        m_pipeline = VulkanRenderer::get()->getPipeline(pipeline);
        m_oneTimeCommandBuffer = VulkanRenderer::get()->getGraphicsCommandBuffers(1)[0];

        VkDevice device = VulkanRenderer::get()->getDevice();
        //TODO: Move this somewhere else
        VkSamplerCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        info.magFilter = VK_FILTER_LINEAR;
        info.minFilter = VK_FILTER_LINEAR;
        info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        info.minLod = -1000;
        info.maxLod = 1000;
        info.maxAnisotropy = 1.0f;
        VkResult res = vkCreateSampler(device, &info, VulkanRenderer::get()->getAllocationCallbacks(), &m_sampler);
        ASSERT_SUCCESS(res, "[IMGUI]: Failed to create sampler");

        //TODO: Maybe lazy load
        assert(m_pipeline != nullptr && "Pipeline should be build before assigning to painter");
    }

    void ImguiPainter::ProcessTexture(RenderScene* scene, ImDrawData* data, ImTextureData* textureData)
    {
        ImTextureStatus status = textureData->Status;

        DeviceManager* deviceManager = VulkanRenderer::get()->getDeviceManager();
        VkDevice device = VulkanRenderer::get()->getDevice();

        //Only supporting this operation in render queue
        uint32_t family = deviceManager->getDeviceDefinition().getQueueOfType(EQueueType::Graphics).getQueueFamily();
        VkQueue queue = deviceManager->getDeviceDefinition().getQueueOfType(EQueueType::Graphics).getVulkanQueueHandle();

        if(status == ImTextureStatus::ImTextureStatus_WantCreate)
        {
            //TODO: Use generic Image.h
            //TODO: Use helper functions in Vulkan renderer after getting back the understandings

            //TODO: Create Texuture
            vulkan::VulkanTextureBuilder builder(m_context);

            VkImageViewCreateInfo imageViewInfo{};
            imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            imageViewInfo.image = VK_NULL_HANDLE;
            imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            imageViewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
            //components field allows to swizzle color channels around. For eg, you can map all channels to red for a monochromatic view
            imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            //subresource range selects mipmap levels and array layers to be accessible to the view
            imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageViewInfo.subresourceRange.baseArrayLayer = 0;
            imageViewInfo.subresourceRange.layerCount = 1;
            imageViewInfo.subresourceRange.baseMipLevel = 0;
            imageViewInfo.subresourceRange.levelCount = 1;

            //Create texture, back memory and create it's image view
            VulkanTexture texture = builder.setHeight(textureData->Height)
            .setWidth(textureData->Width)
            .setFormat(VK_FORMAT_R8G8B8A8_UNORM)
            .setQueueFamilies(1, &family)
            .setUsage(VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT)
            .setMemoryProperty(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
            .setInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
            .setSharingMode(VK_SHARING_MODE_EXCLUSIVE)
            .setTiling(VK_IMAGE_TILING_OPTIMAL)
            .setImageViewInfo(imageViewInfo)
            .setBuildImage()
            .setBackingMemory()
            .setBuildImageView()
            .build();

            //Create descriptor set for it
            VulkanRenderScene* vulkanScene = scene->getRenderSceneImplAs<VulkanRenderScene>();
            VulkanGraphicsPipeline* pipeline = m_pipeline->GetPipelineImplAs<VulkanGraphicsPipeline>();
            
            //Only a single set is supported per layout;
            VkDescriptorSetLayout layouts[] = {pipeline->getDescriptorSetLayout()};

            VkDescriptorSetAllocateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            info.descriptorPool = pipeline->getDescriptorPool();
            info.descriptorSetCount = 1;
            info.pSetLayouts = layouts;

            VkDescriptorSet set = VK_NULL_HANDLE;
            VkResult res = vkAllocateDescriptorSets(VulkanRenderer::get()->getDevice(), &info, &set);
            assert(res == VK_SUCCESS && "Failed to allocate descriptor sets");
            
            //Update the descriptor set with actual image view
            VkDescriptorImageInfo desc_image[1] = {};
            desc_image[0].sampler = m_sampler;
            desc_image[0].imageView = texture.getImageView();
            desc_image[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            VkWriteDescriptorSet write_desc[1] = {};
            write_desc[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write_desc[0].dstSet = set;
            write_desc[0].descriptorCount = 1;
            write_desc[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            write_desc[0].pImageInfo = desc_image;
            vkUpdateDescriptorSets(device, 1, write_desc, 0, nullptr);
            

            ImguiDescriptorSet setInfo {texture, set};
            m_descriptorInfo.pushBack(setInfo);
            textureData->SetTexID(*texture.getImageRef());
        }

        if(status == ImTextureStatus::ImTextureStatus_WantCreate || status == ImTextureStatus::ImTextureStatus_WantDestroy)
        {
            //Update the descriptors
            //Should move the pixel data to staging buffer and then to image view
            const int upload_x = (textureData->Status == ImTextureStatus_WantCreate) ? 0 : textureData->UpdateRect.x;
            const int upload_y = (textureData->Status == ImTextureStatus_WantCreate) ? 0 : textureData->UpdateRect.y;
            const int upload_w = (textureData->Status == ImTextureStatus_WantCreate) ? textureData->Width : textureData->UpdateRect.w;
            const int upload_h = (textureData->Status == ImTextureStatus_WantCreate) ? textureData->Height : textureData->UpdateRect.h;
            
            VkDeviceMemory stagingGpuMemory;
            VkBuffer stagingBuffer;
            //VulkanRenderer::get()->createBuffer(textureData->GetSizeInBytes(), textureData->GetPixels(), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
            //, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingGpuMemory, stagingBuffer);
            
            const uint32_t alignment = 64;
            size_t upload_pitch = upload_w * textureData->BytesPerPixel;
            size_t upload_size = (upload_h * upload_pitch + alignment - 1) & ~(alignment - 1); //TODO: Align data?????

            //Create buffer and backing memory
            VulkanRenderer::get()->createBuffer(upload_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT
            , VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, stagingGpuMemory, stagingBuffer);

            //Copy image data to buffer
            char* map;
            VkResult res = vkMapMemory(device, stagingGpuMemory, 0, upload_size, 0, (void**)(&map));
            ASSERT_SUCCESS(res, "Failed to map memory");
            for (int y = 0; y < upload_h; y++)
            {
                memcpy(map + upload_pitch * y, textureData->GetPixelsAt(upload_x, upload_y + y), (size_t)upload_pitch);
            }
            VkMappedMemoryRange range[1] = {};
            range[0].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
            range[0].memory = stagingGpuMemory;
            range[0].size = upload_size;
            res = vkFlushMappedMemoryRanges(device, 1, range);
            ASSERT_SUCCESS(res, "[IMGUI]Failed to flush memory ranges to GPU");
            vkUnmapMemory(device, stagingGpuMemory);

            
            //Move image data from buffer to image view
            VkCommandBufferBeginInfo info{};
            info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            vkBeginCommandBuffer(m_oneTimeCommandBuffer, &info);

            VkBufferMemoryBarrier bBar {};
            bBar.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
            bBar.buffer = stagingBuffer;
            bBar.size = upload_size;
            bBar.offset = 0;
            bBar.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            bBar.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            bBar.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
            bBar.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            
            VkImageMemoryBarrier iBar{};
            iBar.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            iBar.image = textureData->GetTexID();
            iBar.oldLayout = (textureData->Status == ImTextureStatus_WantCreate) ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            iBar.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            iBar.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            iBar.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            iBar.srcAccessMask = (textureData->Status == ImTextureStatus_WantCreate) ? VK_ACCESS_NONE : VK_ACCESS_SHADER_READ_BIT;
            iBar.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            iBar.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            iBar.subresourceRange.levelCount = 1;
            iBar.subresourceRange.layerCount = 1;

            //Transition image to receive data from buffer
            vkCmdPipelineBarrier(m_oneTimeCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_HOST_BIT,
                VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 1, &bBar, 1, &iBar);

            //Perform Copy
            VkBufferImageCopy cpy{};
            cpy.imageOffset.x = upload_x;
            cpy.imageOffset.y = upload_y;
            cpy.imageExtent.width = upload_w;
            cpy.imageExtent.height = upload_h;
            cpy.imageExtent.depth = 1;
            cpy.imageSubresource.layerCount = 1;
            cpy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

            vkCmdCopyBufferToImage(m_oneTimeCommandBuffer, stagingBuffer, textureData->GetTexID(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &cpy);
            
            iBar.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            iBar.image = textureData->GetTexID();
            iBar.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            iBar.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            iBar.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            iBar.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            iBar.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            iBar.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            iBar.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            iBar.subresourceRange.levelCount = 1;
            iBar.subresourceRange.layerCount = 1;
            
            //transition image to shader read
            vkCmdPipelineBarrier(m_oneTimeCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &iBar);


            //Finally submit commands
            VkSubmitInfo end_info = {};
            end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            end_info.commandBufferCount = 1;
            end_info.pCommandBuffers = &m_oneTimeCommandBuffer;
            res = vkEndCommandBuffer(m_oneTimeCommandBuffer);
            ASSERT_SUCCESS(res, "[IMGUI]: Failed to end command buffer");
            res = vkQueueSubmit(queue, 1, &end_info, VK_NULL_HANDLE);
            ASSERT_SUCCESS(res, "[IMGUI]: Failed to submit queue");

            //Not optimal. Address this once everything starts to fall in place
            vkDeviceWaitIdle(device);

            //Cleanup buffer resources
            vkDestroyBuffer(device, stagingBuffer, VulkanRenderer::get()->getAllocationCallbacks());
            vkFreeMemory(device, stagingGpuMemory, VulkanRenderer::get()->getAllocationCallbacks());
            textureData->SetStatus(ImTextureStatus_OK);
        }

        //If it hasn't been used for arbitraty amount of time
        if (textureData->Status == ImTextureStatus_WantDestroy && textureData->UnusedFrames >= 3)
        {
            for(size_t i = 0; i < m_descriptorInfo.getSize(); ++i)
            {
                ImguiDescriptorSet& info = m_descriptorInfo[i];
                if(info.texture.getHandle() == textureData->GetTexID())
                {
                    VulkanGraphicsPipeline* pipeline = m_pipeline->GetPipelineImplAs<VulkanGraphicsPipeline>();
                    info.texture.destroy();
                    vkFreeDescriptorSets(device, pipeline->getDescriptorPool(), 1, &info.set);
                    break;
                }
            }
        }
    }

    void ImguiPainter::setupRenderState(RenderScene* scene, ImDrawData* data, VkCommandBuffer cmdBuffer)
    {
        VulkanRenderScene* vulkanScene = scene->getRenderSceneImplAs<VulkanRenderScene>();
        if (data->TotalVtxCount > 0)
        {
            VkBuffer vertex_buffers[1] = { m_vertexBuffer.buffer };
            VkDeviceSize vertex_offset[1] = { 0 };
            vkCmdBindVertexBuffers(cmdBuffer, 0, 1, vertex_buffers, vertex_offset);
            vkCmdBindIndexBuffer(cmdBuffer, m_indexBuffer.buffer, 0, sizeof(ImDrawIdx) == 2 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32);
        }

        // Setup viewport:
        {
            VkViewport viewport;
            viewport.x = (float)vulkanScene->getRenderOffset().x;
            viewport.y = (float)vulkanScene->getRenderOffset().y;
            viewport.width = (float)vulkanScene->getRenderExtent().width;
            viewport.height = (float)vulkanScene->getRenderExtent().height;
            viewport.minDepth = 0;
            viewport.maxDepth = 1;
            vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);
        }

        // Setup scale and translation:
        // Our visible imgui space lies from draw_data->DisplayPps (top left) to draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayPos is (0,0) for single viewport apps.
        {
            VulkanGraphicsPipeline* pipeline = m_pipeline->GetPipelineImplAs<VulkanGraphicsPipeline>();
            float scale[2];
            scale[0] = 2.0f / data->DisplaySize.x;
            scale[1] = 2.0f / data->DisplaySize.y;
            float translate[2];
            translate[0] = -1.0f - data->DisplayPos.x * scale[0];
            translate[1] = -1.0f - data->DisplayPos.y * scale[1];
            vkCmdPushConstants(cmdBuffer, pipeline->getPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, sizeof(float) * 0, sizeof(float) * 2, scale);
            vkCmdPushConstants(cmdBuffer, pipeline->getPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, sizeof(float) * 2, sizeof(float) * 2, translate);
        }
    }

    size_t align(size_t size)
    {
        return (size + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
    }

    void ImguiPainter::render(RenderScene* scene)
    {
        VulkanGraphicsPipeline* pipeline = m_pipeline->GetPipelineImplAs<VulkanGraphicsPipeline>();

        VkDevice device = VulkanRenderer::get()->getDevice();
        //TODO: Move all of this to platform agnostic code
        ImDrawData* data = ImguiHandler::Get()->RenderAndGetDrawData();

        int fb_width = (int)(data->DisplaySize.x * data->FramebufferScale.x);
        int fb_height = (int)(data->DisplaySize.y * data->FramebufferScale.y);
        if (fb_width <= 0 || fb_height <= 0)
            return;

        //Prepares the textures
        for(int i = 0; i < data->Textures->size(); ++i)
        {
            ImTextureData* textureData = (*data->Textures)[i];
            ProcessTexture(scene, data, textureData);
        }

        //TODO: Handle binding data to vertex/fragment buffers
        VulkanRenderScene* vulkanScene = scene->getRenderSceneImplAs<VulkanRenderScene>();
        VkCommandBuffer cmdBuffer = vulkanScene->getSceneParams().commandBuffer;

        if (data->TotalVtxCount > 0)
        {
            // Create or resize the vertex/index buffers
            size_t vertex_size = align(data->TotalVtxCount * sizeof(ImDrawVert));
            size_t index_size = align(data->TotalIdxCount * sizeof(ImDrawIdx));
            if (m_vertexBuffer.buffer == VK_NULL_HANDLE || m_vertexBuffer.size < vertex_size)
            {
                VulkanRenderer::get()->createBuffer(vertex_size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
                    , VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, m_vertexBuffer.memory, m_vertexBuffer.buffer);
                m_vertexBuffer.size = vertex_size;
            }
            if (m_indexBuffer.buffer == VK_NULL_HANDLE || m_indexBuffer.size < index_size)
            {
                VulkanRenderer::get()->createBuffer(index_size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT
                    , VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, m_indexBuffer.memory, m_indexBuffer.buffer);
                m_indexBuffer.size = index_size;
            }

            // Upload vertex/index data into a single contiguous GPU buffer
            ImDrawVert* vtx_dst = nullptr;
            ImDrawIdx* idx_dst = nullptr;
            VkResult err = vkMapMemory(device
                        , m_vertexBuffer.memory, 0, vertex_size, 0, (void**)&vtx_dst);
            ASSERT_SUCCESS(err, "Failed to map vertex memory");
            err = vkMapMemory(device
                        , m_indexBuffer.memory, 0, index_size, 0, (void**)&idx_dst);
            ASSERT_SUCCESS(err, "Failed to map index memory");

            for (const ImDrawList* draw_list : data->CmdLists)
            {
                memcpy(vtx_dst, draw_list->VtxBuffer.Data, draw_list->VtxBuffer.Size * sizeof(ImDrawVert));
                memcpy(idx_dst, draw_list->IdxBuffer.Data, draw_list->IdxBuffer.Size * sizeof(ImDrawIdx));
                vtx_dst += draw_list->VtxBuffer.Size;
                idx_dst += draw_list->IdxBuffer.Size;
            }
            VkMappedMemoryRange range[2] = {};
            range[0].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
            range[0].memory = m_vertexBuffer.memory;
            range[0].size = VK_WHOLE_SIZE;
            range[1].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
            range[1].memory = m_indexBuffer.memory;
            range[1].size = VK_WHOLE_SIZE;
            err = vkFlushMappedMemoryRanges(device, 2, range);
            ASSERT_SUCCESS(err, "failed to flush vertex/index buffers");

            vkUnmapMemory(device, m_vertexBuffer.memory);
            vkUnmapMemory(device, m_indexBuffer.memory);
        }
        
        setupRenderState(scene, data, cmdBuffer);

        ImVec2 clip_off = data->DisplayPos;         // (0,0) unless using multi-viewports
        ImVec2 clip_scale = data->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

        // Render command lists
        // (Because we merged all buffers into a single one, we maintain our own offset into them)
        VkDescriptorSet last_desc_set = VK_NULL_HANDLE;
        int global_vtx_offset = 0;
        int global_idx_offset = 0;
        for (const ImDrawList* draw_list : data->CmdLists)
        {
            for (int cmd_i = 0; cmd_i < draw_list->CmdBuffer.Size; cmd_i++)
            {
                const ImDrawCmd* pcmd = &draw_list->CmdBuffer[cmd_i];
                if (pcmd->UserCallback != nullptr)
                {
                    // User callback, registered via ImDrawList::AddCallback()
                    // (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
                    if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
                        setupRenderState(scene, data, cmdBuffer);
                    else
                        pcmd->UserCallback(draw_list, pcmd);
                    last_desc_set = VK_NULL_HANDLE;
                }
                else
                {
                    // Project scissor/clipping rectangles into framebuffer space
                    ImVec2 clip_min((pcmd->ClipRect.x - clip_off.x) * clip_scale.x, (pcmd->ClipRect.y - clip_off.y) * clip_scale.y);
                    ImVec2 clip_max((pcmd->ClipRect.z - clip_off.x) * clip_scale.x, (pcmd->ClipRect.w - clip_off.y) * clip_scale.y);

                    // Clamp to viewport as vkCmdSetScissor() won't accept values that are off bounds
                    if (clip_min.x < 0.0f) { clip_min.x = 0.0f; }
                    if (clip_min.y < 0.0f) { clip_min.y = 0.0f; }
                    if (clip_max.x > fb_width) { clip_max.x = (float)fb_width; }
                    if (clip_max.y > fb_height) { clip_max.y = (float)fb_height; }
                    if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
                        continue;

                    // Apply scissor/clipping rectangle
                    VkRect2D scissor;
                    scissor.offset.x = (int32_t)(clip_min.x);
                    scissor.offset.y = (int32_t)(clip_min.y);
                    scissor.extent.width = (uint32_t)(clip_max.x - clip_min.x);
                    scissor.extent.height = (uint32_t)(clip_max.y - clip_min.y);
                    vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);

                    // Bind DescriptorSet with font or user texture
                    VkDescriptorSet desc_set = VK_NULL_HANDLE;

                    //TODO: Get rid of the loop
                    for(size_t i = 0; i < m_descriptorInfo.getSize(); ++i)
                    {
                        if(m_descriptorInfo[i].texture.getHandle() == pcmd->GetTexID())
                        {
                            desc_set = m_descriptorInfo[i].set;
                        }
                    }

                    if (desc_set != VK_NULL_HANDLE && desc_set != last_desc_set)
                        vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getPipelineLayout(), 0, 1, &desc_set, 0, nullptr);
                    last_desc_set = desc_set;

                    // Draw
                    vkCmdDrawIndexed(cmdBuffer, pcmd->ElemCount, 1, pcmd->IdxOffset + global_idx_offset, pcmd->VtxOffset + global_vtx_offset, 0);
                }
            }
            global_idx_offset += draw_list->IdxBuffer.Size;
            global_vtx_offset += draw_list->VtxBuffer.Size;
        }

        // Note: at this point both vkCmdSetViewport() and vkCmdSetScissor() have been called.
        // Our last values will leak into user/application rendering IF:
        // - Your app uses a pipeline with VK_DYNAMIC_STATE_VIEWPORT or VK_DYNAMIC_STATE_SCISSOR dynamic state
        // - And you forgot to call vkCmdSetViewport() and vkCmdSetScissor() yourself to explicitly set that state.
        // If you use VK_DYNAMIC_STATE_VIEWPORT or VK_DYNAMIC_STATE_SCISSOR you are responsible for setting the values before rendering.
        // In theory we should aim to backup/restore those values but I am not sure this is possible.
        // We perform a call to vkCmdSetScissor() to set back a full viewport which is likely to fix things for 99% users but technically this is not perfect. (See github #4644)
        VkRect2D scissor = { { 0, 0 }, { (uint32_t)fb_width, (uint32_t)fb_height } };
        vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);
    }
    
    void ImguiPainter::preRender(RenderScene* scene, uint32_t stage)
    {

    }
    void ImguiPainter::postRender()
    {

    }

}