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
        void emit(uint32_t part_count, std::pair<vec2f, vec2f> pos, std::pair<float, float> speed, std::pair<float, float> lifetime, std::pair<float, float> angle, vec4f c);
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
        static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
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

    void m_setupStorageBuffers(Device& device);
    void m_setupEmitBuffers(Device& device);
    void m_setupDescriptorsLayout(Device& device);
    void m_createPipeline(Device& device, VkRenderPass render_pass, VkDescriptorSetLayout computeSetLayout);
public:
    void compute(const FrameInfo& frame_info);
    void render(const FrameInfo& frame_info);
    ~ParticleSystem();
    ParticleSystem(Device& device, VkRenderPass render_pass, VkDescriptorSetLayout global_compute_layout, float aspect);
};
}

#endif // !DEBUG
