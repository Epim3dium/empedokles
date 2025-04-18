#ifndef EMP_PARTICLE_SYSTEM_HPP
#define EMP_PARTICLE_SYSTEM_HPP
#include <vulkan/vulkan_core.h>
#include <iostream>
#include <random>
#include "debug/log.hpp"
#include "graphics/frame_info.hpp"
#include "math/math_defs.hpp"
#include "vulkan/buffer.hpp"
#include "vulkan/descriptors.hpp"
#include "vulkan/device.hpp"
#include "vulkan/pipeline.hpp"
#include "vulkan/swap_chain.hpp"
namespace emp {

class ParticleSystem {
public:
    static constexpr uint32_t MAX_PARTICLE_COUNT = 65536U;
    //float to 4
    //vec2 to 8
    //vec3/vec4 to 16
    struct alignas(16) ParticleEmitData {
        uint32_t count; // 4, 4
        float _pad0;// 4,8
        vec2f position_min;// 8, 16
        vec2f position_max;// 8, 24

        float speed_min;//4, 28
        float speed_max;//4, 32

        float lifetime_min;//4, 36
        float lifetime_max;//4, 40

        float angle_min;//4, 44
        float angle_max;//4, 48

        //48 % 16 == 0 C:
        vec4f color;
    };
    static constexpr uint32_t MAX_EMIT_CALLS = 16U;
    struct alignas(64) EmitQueue {
        ParticleEmitData calls[MAX_EMIT_CALLS];
        uint32_t call_count = 0;
        uint32_t work_start = 0;
        uint32_t work_end = 0;
        uint32_t max_particles = MAX_PARTICLE_COUNT; // pad to 16 bytes
        void emit(uint32_t part_count, std::pair<vec2f, vec2f> pos, std::pair<float, float> speed, std::pair<float, float> lifetime,
                  std::pair<float, float> angle, vec4f c) 
        {
            if(call_count >= MAX_EMIT_CALLS) {
                return;
            }
            EMP_LOG_DEBUG << work_end;
            work_end += part_count;
            auto idx = call_count++;
            calls[idx].count = part_count;
            calls[idx].position_min = pos.first;
            calls[idx].position_max = pos.second;
            calls[idx].lifetime_min = lifetime.first;
            calls[idx].lifetime_max = lifetime.second;
            calls[idx].angle_min = angle.first;
            calls[idx].angle_max = angle.second;
            calls[idx].speed_min = speed.first;
            calls[idx].speed_max = speed.second;
            calls[idx].color = c;
        }
        void reset() {
            work_start = work_end;
            call_count = 0;
        }
    };
    struct alignas(16U) ParticleData {
        vec2f position;
        vec2f velocity;
        vec4f color;
        float lifetime;
        static std::vector<VkVertexInputBindingDescription> getBindingDescriptions() {
            std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
            bindingDescriptions[0].binding = 0;
            bindingDescriptions[0].stride = sizeof(ParticleData);
            bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            return bindingDescriptions;
        }
        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
            std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

            int location = 0;
            int binding = 0;
            for(auto info : {
                std::pair(VK_FORMAT_R32G32_SFLOAT, offsetof(ParticleData, position)),
                std::pair(VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(ParticleData, color))})
            {
                attributeDescriptions.push_back({});
                auto& attr = attributeDescriptions.back();
                attr.binding = binding;
                attr.location = location++;
                attr.offset = info.second;
                attr.format = info.first;
            }

            return attributeDescriptions;
        }
    };
    static_assert(sizeof(ParticleData) == 48, "Wrong struct size");
private:
    VkPipelineLayout compute_pipeline_layout{};
    VkPipelineLayout graphics_pipeline_layout{};
    std::unique_ptr<Pipeline> compute_pipeline;
    std::unique_ptr<Pipeline> graphics_pipeline;

    std::vector<std::unique_ptr<Buffer>> SSBO_buffers;
    std::vector<std::unique_ptr<Buffer>> emit_buffers;

    std::unique_ptr<DescriptorSetLayout> SSBO_layout;
    std::unique_ptr<DescriptorSetLayout> emit_buffer_layout;

    std::unique_ptr<DescriptorPool> compute_pool;
    std::vector<VkDescriptorSet> SSBO_descriptors;
    std::vector<VkDescriptorSet> emit_buffer_descriptors;
    Device& m_device;
    EmitQueue m_emit_queue;

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
            particle.lifetime = rndDist(rndEngine) * 2 + 2.f;
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
        for(auto& buf : SSBO_buffers) {
            device.copyBuffer(
                stagingBuffer.getBuffer(), buf->getBuffer(), stagingBuffer.getBufferSize()
            );
        }
        stagingBuffer.unmap();
        m_emit_queue.emit(2000, {{-0.1, -0.1}, {0.1, 0.1}}, {0, 0.3}, {6, 7}, {0, 6.282}, {1, 0, 0, 1});
        for(auto& b : emit_buffers) {
            b->writeToBuffer(&m_emit_queue);
            b->flush();
        }
    }

    void m_setupStorageBuffers(Device& device) {
        SSBO_buffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
        for(int i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
            SSBO_buffers[i] = std::make_unique<Buffer>(device,
                sizeof(ParticleData),
                MAX_PARTICLE_COUNT,
                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                    VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        }
    }
    void m_setupEmitBuffers(Device& device) {
        emit_buffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
        for(int i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
            emit_buffers[i] = std::make_unique<Buffer>(device,
                sizeof(EmitQueue),
                1U,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            emit_buffers[i]->map();
        }
    }
    void m_setupDescriptorsLayout(Device& device) {
        compute_pool = DescriptorPool::Builder(device)
                           .setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT * 2U)
                           .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 4)
                           .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2)
                           .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2)
                           .build();
        SSBO_layout =
            DescriptorSetLayout::Builder(device)
                .addBinding(0,
                    VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                    VK_SHADER_STAGE_COMPUTE_BIT)
                .addBinding(1,
                    VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                    VK_SHADER_STAGE_COMPUTE_BIT)
                .build();
        SSBO_descriptors.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

        for(int i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
            DescriptorWriter desc_writer(*SSBO_layout, *compute_pool);

            VkDescriptorBufferInfo storageBufferInfoLastFrame{};
            storageBufferInfoLastFrame.buffer = SSBO_buffers[(i + 1) % SwapChain::MAX_FRAMES_IN_FLIGHT]->getBuffer();
            storageBufferInfoLastFrame.offset = 0;
            storageBufferInfoLastFrame.range = sizeof(ParticleData) * MAX_PARTICLE_COUNT;
            desc_writer.writeBuffer(0, &storageBufferInfoLastFrame);

            VkDescriptorBufferInfo storageBufferInfoCurrentFrame{};
            storageBufferInfoCurrentFrame.buffer = SSBO_buffers[i]->getBuffer();
            storageBufferInfoCurrentFrame.offset = 0;
            storageBufferInfoCurrentFrame.range = sizeof(ParticleData) * MAX_PARTICLE_COUNT;
            desc_writer.writeBuffer(1, &storageBufferInfoCurrentFrame);
            desc_writer.build(SSBO_descriptors[i]);
        }
        emit_buffer_layout =
            DescriptorSetLayout::Builder(device)
                .addBinding(0,
                    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    VK_SHADER_STAGE_COMPUTE_BIT)
                .build();
        emit_buffer_descriptors.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
        for(int i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
            DescriptorWriter desc_writer(*emit_buffer_layout, *compute_pool);
            VkDescriptorBufferInfo queryBuffer_info{};
            queryBuffer_info.buffer = emit_buffers[i]->getBuffer();
            queryBuffer_info.offset = 0;
            queryBuffer_info.range = sizeof(EmitQueue);
            desc_writer.writeBuffer(0, &queryBuffer_info);
            desc_writer.build(emit_buffer_descriptors[i]);
        }
    }
    void m_createPipeline(Device& device, VkRenderPass render_pass, VkDescriptorSetLayout computeSetLayout) {
        {
            PipelineConfigInfo config;
            VkDescriptorSetLayout layouts[] = {
                computeSetLayout,
                SSBO_layout->getDescriptorSetLayout(),
                emit_buffer_layout->getDescriptorSetLayout()
            };
            // Create pipeline layout
            VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
            pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayoutInfo.setLayoutCount = 3;
            pipelineLayoutInfo.pSetLayouts = layouts;

            vkCreatePipelineLayout(
                device.device(), &pipelineLayoutInfo, nullptr, &compute_pipeline_layout);

            config.pipelineLayout = compute_pipeline_layout;
            compute_pipeline = std::make_unique<Pipeline>(
                device, "../assets/shaders/particle_update.comp.spv", config);
        }
        {
            PipelineConfigInfo config;
            Pipeline::defaultPipelineConfigInfo(config);
            config.renderPass = render_pass;
            config.attributeDescriptions = ParticleData::getAttributeDescriptions();
            config.bindingDescriptions = ParticleData::getBindingDescriptions();
            config.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;

            VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
            pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayoutInfo.setLayoutCount = 0;
            vkCreatePipelineLayout(
                device.device(), &pipelineLayoutInfo, nullptr, &graphics_pipeline_layout);

            config.pipelineLayout = graphics_pipeline_layout;
            graphics_pipeline = std::make_unique<Pipeline>(
                device, "../assets/shaders/particle_draw.vert.spv", "../assets/shaders/particle_draw.frag.spv", config);
        }

    }
public:
    void compute(const FrameInfo& frame_info) {
        emit_buffers[frame_info.frameIndex]->writeToBuffer(&m_emit_queue);
        emit_buffers[frame_info.frameIndex]->flush();
        m_emit_queue.reset();
        m_emit_queue.emit(200, {{-0.1, -0.1}, {0.1, 0.1}}, {0, 0.3}, {6, 7}, {0, 6.282}, {1, 0, 0, 1});

        auto frame_index = frame_info.frameIndex;
        compute_pipeline->bind(frame_info.commandBuffer);

        const int desc_nb = 3;
        VkDescriptorSet descriptors[desc_nb] = {
            frame_info.globalDescriptorSet,
            SSBO_descriptors[frame_index],
            emit_buffer_descriptors[frame_index]
        };
        compute_pipeline->bindDescriptorSets(frame_info.commandBuffer, descriptors, 0, desc_nb);
        vkCmdDispatch(frame_info.commandBuffer, MAX_PARTICLE_COUNT / 256, 1, 1);
    }
    void render(const FrameInfo& frame_info) {
        auto frame_index = frame_info.frameIndex;
        graphics_pipeline->bind(frame_info.commandBuffer);

        VkDeviceSize offsets[] = {0};
        VkBuffer buffers[] = {SSBO_buffers[frame_index]->getBuffer()};

        vkCmdBindVertexBuffers(frame_info.commandBuffer, 0, 1, buffers, offsets);
        vkCmdDraw(frame_info.commandBuffer, MAX_PARTICLE_COUNT, 1, 0, 0);

        // frame_info.
    }
    ~ParticleSystem() {
        vkDestroyPipelineLayout(m_device.device(), compute_pipeline_layout, NULL);
        vkDestroyPipelineLayout(m_device.device(), graphics_pipeline_layout, NULL);
    }
    ParticleSystem(Device& device, VkRenderPass render_pass, VkDescriptorSetLayout global_layout, float aspect) : m_device(device) {
        m_setupStorageBuffers(device);
        m_setupEmitBuffers(device);
        m_initRandomParticles(device, aspect);
        m_setupDescriptorsLayout(device);
        m_createPipeline(device, render_pass, global_layout);
    }
};
}

#endif // !DEBUG
