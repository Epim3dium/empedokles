#pragma once 

#include <vulkan/vulkan_core.h>
#include <memory>
#include "graphics/frame_info.hpp"
#include "graphics/texture.hpp"
#include "vulkan/pipeline.hpp"
#include "vulkan/buffer.hpp"
#include "vulkan/swap_chain.hpp"
#include "vulkan/descriptors.hpp"
namespace emp {
class ComputeDemo {
public:
    Device& m_device;
    std::unique_ptr<DescriptorSetLayout> layoutBinding;
    std::unique_ptr<DescriptorPool> compute_pool;
    const size_t dataCount = 4;
    VkDescriptorSet descriptor_set;
    VkPipelineLayout pipelineLayout{};
    std::unique_ptr<Pipeline> compute_pipeline;
    std::unique_ptr<Buffer> dataBuffer;
    std::vector<float> inputData;

    std::unique_ptr<TextureAsset> output_image;
    TextureAsset* display_image;
    

    ComputeDemo(Device& device) : m_device(device) {
        VkExtent3D extent = {2, 2, 1};
        assert(extent.width * extent.height * extent.depth == dataCount);
        output_image = std::make_unique<TextureAsset>(
            device,
            VK_FORMAT_R8G8B8A8_SRGB,
            extent,
            VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
            VK_SAMPLE_COUNT_1_BIT);


        VkDeviceSize bufferSize = sizeof(float) * dataCount;
        inputData = std::vector<float>(dataCount, 1.0f); // Sample input data: all values set to 2.0
        for(int i = 0; i < dataCount; i += 2) {
            inputData[i] = 512.f;
        }

        // Create buffer
        dataBuffer = std::make_unique<Buffer>(device,
            sizeof(float),
            dataCount,
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        {
            dataBuffer->map();
            memcpy(dataBuffer->getMappedMemory(), inputData.data(), bufferSize);
            dataBuffer->unmap();
        }
        // Create descriptor set layout
        layoutBinding = DescriptorSetLayout::Builder(device)
                            .addBinding(0,
                                VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                VK_SHADER_STAGE_COMPUTE_BIT)
                            .addBinding(1,
                                VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                VK_SHADER_STAGE_COMPUTE_BIT)
                            .build();

        // Descriptor pool and set allocation
        compute_pool = DescriptorPool::Builder(device)
                           .setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
                           .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1)
                           .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1)
                           .build();

        PipelineConfigInfo config;
        {
            // Create pipeline layout
            VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
            pipelineLayoutInfo.sType =
                VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayoutInfo.setLayoutCount = 1;
            auto layouts = layoutBinding->getDescriptorSetLayout();
            pipelineLayoutInfo.pSetLayouts = &layouts;

            vkCreatePipelineLayout(
                device.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout);

            config.pipelineLayout = pipelineLayout;
        }
        compute_pipeline = std::make_unique<Pipeline>(
            device, "../assets/shaders/compute.comp.spv", config);

        {
            auto cmd = device.beginSingleTimeCommands();
            output_image->transitionLayout(cmd, VK_IMAGE_LAYOUT_GENERAL);
            output_image->updateDescriptor();
            device.endSingleTimeCommands(cmd);
        }
        {
            auto bufferInfoDS = dataBuffer->descriptorInfo();
            DescriptorWriter(*layoutBinding, *compute_pool)
                .writeBuffer(0, &bufferInfoDS)
                .writeImage(1, &output_image->getImageInfo())
                .build(descriptor_set);
        }
        display_image = &Texture::create("demo",
            device,
            VK_FORMAT_R8G8B8A8_SRGB,
            extent,
            VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
            VK_SAMPLE_COUNT_1_BIT).texture();

    }
    void performCompute(FrameInfo frame_info) {

        {
            auto cmd = m_device.beginSingleTimeCommands();
                display_image->transitionLayout(cmd,
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
                output_image->transitionLayout(cmd,
                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
            m_device.endSingleTimeCommands(cmd);

        }
        m_device.copyImageToImage(
            output_image->getImage(), display_image->getImage(), 2, 2, 1);
        {
            auto cmd = m_device.beginSingleTimeCommands();
            display_image->transitionLayout(cmd,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            output_image->transitionLayout(cmd,
                VK_IMAGE_LAYOUT_GENERAL);
            m_device.endSingleTimeCommands(cmd);

            display_image->updateDescriptor();
        }

        auto command_buffer = frame_info.commandBuffer;
        compute_pipeline->bind(command_buffer);
        vkCmdBindDescriptorSets(command_buffer,
            VK_PIPELINE_BIND_POINT_COMPUTE,
            pipelineLayout, 0, 1,
            &descriptor_set, 0, nullptr);
        auto workgroup_count_x = 2;

        // output_image.texture().transitionLayout(command_buffer,
        //     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 
        //     VK_IMAGE_LAYOUT_GENERAL);

        vkCmdDispatch(command_buffer, workgroup_count_x , workgroup_count_x , 1); // Dispatch work
        
        return;
        auto& tex = *display_image;

        EMP_LOG_DEBUG << tex.getExtent().width;
        EMP_LOG_DEBUG << tex.getExtent().height;
        EMP_LOG_DEBUG << tex.getExtent().depth;
        auto pixels = tex.getPixelsFromGPU();
        auto h = tex.getExtent().height;
        auto w = tex.getExtent().width;
        std::cout << "\n";
        for(int i = 0; i < h; i += 1) {
            for(int ii = 0; ii < w; ii += 1) {
                auto r = (int)(pixels)[i * w + ii].red;
                auto g = (int)(pixels)[i * w + ii].green;
                auto b = (int)(pixels)[i * w + ii].blue;
                std::cout << "\033[38;2;" << r << ";" << g << ";" << b << "m";
                std::cout << "\u2588";
            }
            std::cout << '\n';
        }
    }
};
};
