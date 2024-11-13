#pragma once 

#include "graphics/sprite.hpp"
#include <vulkan/vulkan_core.h>
#include <memory>
#include "graphics/frame_info.hpp"
#include "graphics/sprite_system.hpp"
#include "graphics/systems/simple_render_system.hpp"
#include "graphics/texture.hpp"
#include "utils/time.hpp"
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
    const size_t dataCount = 256 * 256;
    VkDescriptorSet descriptor_set;
    VkPipelineLayout pipelineLayout{};
    std::unique_ptr<Pipeline> compute_pipeline;
    std::unique_ptr<Buffer> dataBuffer;
    std::vector<float> inputData;

    std::unique_ptr<TextureAsset> output_image;
    TextureAsset* display_image;
    Transform transform;
    vec2f size = {900.f, 900.f};
    std::vector<std::unique_ptr<Buffer>> uboBuffers{
            SwapChain::MAX_FRAMES_IN_FLIGHT
    };

    

void copyImageToImage(VkCommandBuffer command_buffer,
    VkImage src,
    VkImage dst,
    uint32_t width,
    uint32_t height,
    uint32_t layerCount) 
{
    VkImageCopy region {};
    region.extent = {width, height, 1};

    region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.srcSubresource.mipLevel = 0;
    region.srcSubresource.baseArrayLayer = 0;
    region.srcSubresource.layerCount = layerCount;

    region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.dstSubresource.mipLevel = 0;
    region.dstSubresource.baseArrayLayer = 0;
    region.dstSubresource.layerCount = layerCount;

    region.dstOffset = {0, 0, 0};
    region.srcOffset = {0, 0, 0};

    vkCmdCopyImage(command_buffer, src, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}
    void initializeImages(Device& device) {
        VkExtent3D extent = {256, 256, 1};
        assert(extent.width * extent.height * extent.depth == dataCount);
        output_image = std::make_unique<TextureAsset>(
            device,
            VK_FORMAT_R8G8B8A8_SRGB,
            extent,
            VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
            VK_SAMPLE_COUNT_1_BIT);
        display_image = &Texture::create("demo",
            device,
            VK_FORMAT_R8G8B8A8_SRGB,
            extent,
            VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
            VK_SAMPLE_COUNT_1_BIT).texture();
        {
            auto cmd = device.beginSingleTimeCommands();
            output_image->transitionLayout(cmd, VK_IMAGE_LAYOUT_GENERAL);
            output_image->updateDescriptor();
            device.endSingleTimeCommands(cmd);
        }
    }
    void initializeDataBuffer(Device& device) {
        VkDeviceSize bufferSize = sizeof(float) * dataCount;
        inputData = std::vector<float>(dataCount, 0.0f); // Sample input data: all values set to 2.0
        for(int i = 100; i < 150; i++) {
            for(int ii = 100; ii < 150; ii++) {
                inputData[ii + i * 256] = 0.9f;
            }
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
    }
    void createDescriptors(Device& device) {
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
        {
            auto bufferInfoDS = dataBuffer->descriptorInfo();
            DescriptorWriter(*layoutBinding, *compute_pool)
                .writeBuffer(0, &bufferInfoDS)
                .writeImage(1, &output_image->getImageInfo())
                .build(descriptor_set);
        }
    }
    void createPipeline(Device& device) {
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

    }
    ComputeDemo(Device& device) : m_device(device), transform(vec2f(0, 0), 0.f, vec2f(1.f, 1.f)) {
        transform.syncWithChange();
        initializeImages(m_device);
        initializeDataBuffer(m_device);
        createDescriptors(m_device);
        createPipeline(m_device);
        initializeUBO(m_device);
    }
    void initializeUBO(Device& device) {
        int alignment = std::lcm(
                device.properties.limits.nonCoherentAtomSize,
                device.properties.limits.minUniformBufferOffsetAlignment
        );
        for (auto& uboBuffer : uboBuffers) {
            uboBuffer = std::make_unique<Buffer>(
                    device,
                    sizeof(SpriteInfo),
                    1U,
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                    alignment
            );
            uboBuffer->map();
        }
    }
    void updateUBO(int frameIndex) {
        for (auto& uboBuffer : uboBuffers) {
            SpriteInfo data{};

            data.model_matrix = transform.global();
            data.size_matrix = glm::scale(
                    glm::mat4{1.f}, {size.x, size.y, 1.f}
            );

            uboBuffers[frameIndex]->writeToBuffer(&data);
        }
    }
    void render(FrameInfo& frame_info, SimpleRenderSystem& simple_rend_system) {
        updateUBO(frame_info.frameIndex);
        static const std::set<Entity> singleton {0};
        simple_rend_system.render(
                frame_info,
                singleton,
                [this](DescriptorWriter& desc_writer,
                       int frame_index,
                       const Entity&) -> VkDescriptorSet {
                    static VkDescriptorBufferInfo buf_info;
                    buf_info = uboBuffers[frame_index]->descriptorInfo();
                    auto& image_info = output_image->getImageInfo();
                    desc_writer.writeBuffer(0, &buf_info);
                    desc_writer.writeImage(1, &image_info);
                    VkDescriptorSet result;
                    desc_writer.build(result);
                    return result;
                },
                [](const VkCommandBuffer& command_buf, const Entity& entity) {
                    Sprite::bind(command_buf);
                    Sprite::draw(command_buf);
                }
        );
    }
    void performCompute(FrameInfo frame_info) {
        auto command_buffer = frame_info.commandBuffer;

        {
                display_image->transitionLayout(command_buffer,
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
                output_image->transitionLayout(command_buffer,
                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

        }
        copyImageToImage(command_buffer,
            output_image->getImage(), display_image->getImage(), 256, 256, 1);
        {
            display_image->transitionLayout(command_buffer,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            output_image->transitionLayout(command_buffer,
                VK_IMAGE_LAYOUT_GENERAL);
        }
        display_image->updateDescriptor();

        compute_pipeline->bind(command_buffer);
        vkCmdBindDescriptorSets(command_buffer,
            VK_PIPELINE_BIND_POINT_COMPUTE,
            pipelineLayout, 0, 1,
            &descriptor_set, 0, nullptr);

        auto workgroup_count = 16;
        vkCmdDispatch(command_buffer, workgroup_count , workgroup_count , 1); // Dispatch work
        
        return;
        auto& tex = *display_image;

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
