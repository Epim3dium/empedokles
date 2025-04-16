#ifndef EMP_PARTICLE_SYSTEM_HPP
#define EMP_PARTICLE_SYSTEM_HPP
#include <vulkan/vulkan_core.h>
#include <iostream>
#include <random>
#include "graphics/frame_info.hpp"
#include "math/math_defs.hpp"
#include "vulkan/buffer.hpp"
#include "vulkan/descriptors.hpp"
#include "vulkan/device.hpp"
#include "vulkan/pipeline.hpp"
#include "vulkan/swap_chain.hpp"
namespace emp {
struct ParticleData {
    vec2f position;
    vec2f velocity;
    vec4f color;
    static std::vector<VkVertexInputBindingDescription> getBindingDescriptions() {
        std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
        bindingDescriptions[0].binding = 0;
        bindingDescriptions[0].stride = sizeof(ParticleData);
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescriptions;
    }
    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(ParticleData, position);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(ParticleData, color);

        return attributeDescriptions;
    }
};
struct ComputeUBO {
    float deltaTime;
};
class ParticleSystem {
public:
    static constexpr uint32_t MAX_PARTICLE_COUNT = 65536U;
private:
    VkPipelineLayout pipeline_layout{};
    std::unique_ptr<Pipeline> compute_pipeline;
    std::unique_ptr<Pipeline> graphics_pipeline;
    std::vector<std::unique_ptr<Buffer>> uniformBuffers;
    std::vector<std::unique_ptr<Buffer>> shaderStorageBuffers;

    std::unique_ptr<DescriptorSetLayout> compute_system_layout;
    std::unique_ptr<DescriptorSetLayout> graphics_system_layout;

    std::unique_ptr<DescriptorPool> compute_pool;
    std::vector<VkDescriptorSet> descriptorBufferSets;

    void m_initRandomParticles(Device& device, float aspect) {
        std::vector<ParticleData> particles(MAX_PARTICLE_COUNT);
        std::default_random_engine rndEngine((unsigned)time(nullptr));
        std::uniform_real_distribution<float> rndDist(0.0f, 1.0f);
        for(auto& particle : particles) {
            float r = 0.25f * sqrt(rndDist(rndEngine));
            float theta = rndDist(rndEngine) * 2 * 3.14159265358979323846;
            float x = r * cos(theta) / aspect;
            float y = r * sin(theta);
            particle.position = glm::vec2(x, y);
            particle.velocity = glm::normalize(glm::vec2(x,y)) * 0.1f;
            particle.color = glm::vec4(rndDist(rndEngine), rndDist(rndEngine), rndDist(rndEngine), 1.0f);
        }
        Buffer stagingBuffer{
            device,
            sizeof(ParticleData),
            MAX_PARTICLE_COUNT,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        };

        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void*)particles.data());
        for(auto& buf : shaderStorageBuffers) {
            device.copyBuffer(
                stagingBuffer.getBuffer(), buf->getBuffer(), stagingBuffer.getBufferSize()
            );
        }
        stagingBuffer.unmap();
    }

    void m_setupStorageBuffers(Device& device) {
        shaderStorageBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
        uniformBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
        for(int i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
            shaderStorageBuffers[i] = 
                std::make_unique<Buffer>(device, sizeof(ParticleData), MAX_PARTICLE_COUNT, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            uniformBuffers[i] = std::make_unique<Buffer>(
                    device,
                    sizeof(ComputeUBO),
                    1,
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
            );
        }
    }
    void m_setupDescriptorsLayout(Device& device) {
        compute_pool = DescriptorPool::Builder(device)
                           .setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
                           .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2)
                           .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2)
                           .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2)
                           .build();
        compute_system_layout =
            DescriptorSetLayout::Builder(device)
                .addBinding(0,
                    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    VK_SHADER_STAGE_COMPUTE_BIT)
                .addBinding(1,
                    VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                    VK_SHADER_STAGE_COMPUTE_BIT)
                .addBinding(2,
                    VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                    VK_SHADER_STAGE_COMPUTE_BIT)
                .build();
        graphics_system_layout =
            DescriptorSetLayout::Builder(device)
                // .addBinding(0,
                //     VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                //     VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
                .build();
        descriptorBufferSets.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

        for(int i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
            DescriptorWriter desc_writer(*compute_system_layout, *compute_pool);

            VkDescriptorBufferInfo uniformBufferInfo{};
            uniformBufferInfo.buffer = uniformBuffers[i]->getBuffer();
            uniformBufferInfo.offset = 0;
            uniformBufferInfo.range = sizeof(ComputeUBO);
            desc_writer.writeBuffer(0, &uniformBufferInfo);

            VkDescriptorBufferInfo storageBufferInfoLastFrame{};
            storageBufferInfoLastFrame.buffer = shaderStorageBuffers[(i + 1) % SwapChain::MAX_FRAMES_IN_FLIGHT]->getBuffer();
            storageBufferInfoLastFrame.offset = 0;
            storageBufferInfoLastFrame.range = sizeof(ParticleData) * MAX_PARTICLE_COUNT;
            desc_writer.writeBuffer(1, &storageBufferInfoLastFrame);

            VkDescriptorBufferInfo storageBufferInfoCurrentFrame{};
            storageBufferInfoCurrentFrame.buffer = shaderStorageBuffers[i]->getBuffer();
            storageBufferInfoCurrentFrame.offset = 0;
            storageBufferInfoCurrentFrame.range = sizeof(ParticleData) * MAX_PARTICLE_COUNT;
            desc_writer.writeBuffer(2, &storageBufferInfoCurrentFrame);
            desc_writer.build(descriptorBufferSets[i]);
        }
    }
    void m_createPipeline(Device& device, VkRenderPass render_pass) {
        {
            PipelineConfigInfo config;
            auto layouts = compute_system_layout->getDescriptorSetLayout();
            // Create pipeline layout
            VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
            pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayoutInfo.setLayoutCount = 1;
            pipelineLayoutInfo.pSetLayouts = &layouts;

            vkCreatePipelineLayout(
                device.device(), &pipelineLayoutInfo, nullptr, &pipeline_layout);

            config.pipelineLayout = pipeline_layout;
            compute_pipeline = std::make_unique<Pipeline>(
                device, "../assets/shaders/particle_update.comp.spv", config);
        }
        {
            PipelineConfigInfo config;
            auto layouts = graphics_system_layout->getDescriptorSetLayout();
            Pipeline::defaultPipelineConfigInfo(config);
            config.renderPass = render_pass;
            config.attributeDescriptions = ParticleData::getAttributeDescriptions();
            config.bindingDescriptions = ParticleData::getBindingDescriptions();
            config.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;

            VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
            pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayoutInfo.setLayoutCount = 1;
            pipelineLayoutInfo.pSetLayouts = &layouts;
            vkCreatePipelineLayout(
                device.device(), &pipelineLayoutInfo, nullptr, &pipeline_layout);

            config.pipelineLayout = pipeline_layout;
            graphics_pipeline = std::make_unique<Pipeline>(
                device, "../assets/shaders/particle_draw.vert.spv", "../assets/shaders/particle_draw.frag.spv", config);
        }

    }
public:
    void compute(const FrameInfo& frame_info) {
        auto frame_index = frame_info.frameIndex;
        ComputeUBO updated{frame_info.frameTime};
        uniformBuffers[frame_index]->map();
        uniformBuffers[frame_index]->writeToBuffer(&updated);
        uniformBuffers[frame_index]->unmap();
        compute_pipeline->bind(frame_info.commandBuffer);
        compute_pipeline->bindDescriptorSets(frame_info.commandBuffer, &descriptorBufferSets[frame_index], 0);
        vkCmdDispatch(frame_info.commandBuffer, MAX_PARTICLE_COUNT / 256, 1, 1);
    }
    void render(const FrameInfo& frame_info) {
        auto frame_index = frame_info.frameIndex;
        graphics_pipeline->bind(frame_info.commandBuffer);

        VkDeviceSize offsets[] = {0};
        VkBuffer buffers[] = {shaderStorageBuffers[frame_index]->getBuffer()};

        vkCmdBindVertexBuffers(frame_info.commandBuffer, 0, 1, buffers, offsets);
        vkCmdDraw(frame_info.commandBuffer, MAX_PARTICLE_COUNT, 1, 0, 0);

        // frame_info.
    }
    ParticleSystem(Device& device, VkRenderPass render_pass, float aspect) {
        m_setupStorageBuffers(device);
        m_initRandomParticles(device, aspect);
        m_setupDescriptorsLayout(device);
        m_createPipeline(device, render_pass);
    }
};
}

#endif // !DEBUG
