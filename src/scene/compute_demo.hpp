#pragma once 

#include <vulkan/vulkan_core.h>
#include <memory>
#include "vulkan/pipeline.hpp"
#include "vulkan/buffer.hpp"
#include "vulkan/swap_chain.hpp"
#include "vulkan/descriptors.hpp"
namespace emp {
class ComputeDemo {
public:
    std::unique_ptr<DescriptorSetLayout> layoutBinding;
    std::unique_ptr<DescriptorPool> compute_pool;
    const size_t dataCount = 1'000'000;
    VkDescriptorSet descriptor_set;
    VkPipelineLayout pipelineLayout{};
    std::unique_ptr<Pipeline> compute_pipeline;
    std::unique_ptr<Buffer> dataBuffer;
    std::vector<float> inputData;
    ComputeDemo(Device& device) {
        VkDeviceSize bufferSize = sizeof(float) * dataCount;
        inputData = std::vector<float>(
            dataCount, 2.0f); // Sample input data: all values set to 2.0
        for (int i = 0; i < inputData.size(); i += 2) {
            inputData[i] *= 2.f;
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
                            .build();

        // Descriptor pool and set allocation
        compute_pool = DescriptorPool::Builder(device)
                           .setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
                           .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1)
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
            VkDescriptorBufferInfo bufferInfoDS{};
            bufferInfoDS.buffer = dataBuffer->getBuffer();
            bufferInfoDS.offset = 0;
            bufferInfoDS.range = bufferSize;

            DescriptorWriter(*layoutBinding, *compute_pool)
                .writeBuffer(0, &bufferInfoDS)
                .build(descriptor_set);
        }
    }
    void performCompute(VkCommandBuffer commandBuffer) {

        compute_pipeline->bind(commandBuffer);
        vkCmdBindDescriptorSets(commandBuffer,
            VK_PIPELINE_BIND_POINT_COMPUTE,
            pipelineLayout, 0, 1,
            &descriptor_set, 0, nullptr);
        vkCmdDispatch(commandBuffer, (dataCount + 63) / 64, 1, 1); // Dispatch work

        // // Map memory again to read back data
        // dataBuffer->map();
        // float* resultData = reinterpret_cast<float*>(dataBuffer->getMappedMemory());
        // static int ticks = 4;
        // ticks++;
        // EMP_LOG_INTERVAL(DEBUG, 1.f) << "ResultCl: " << ticks;
        // EMP_LOG_INTERVAL(DEBUG, 1.f) << "Result0: " << resultData[0];
        // EMP_LOG_INTERVAL(DEBUG, 1.f) << "Result5: " << resultData[500'000];
        // EMP_LOG_INTERVAL(DEBUG, 1.f) << "Result10: " << resultData[1'000'000 - 2];
        // dataBuffer->unmap();
    }
};
};
