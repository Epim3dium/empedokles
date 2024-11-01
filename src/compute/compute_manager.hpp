#ifndef EMP_COMPUTE_MANAGER_HPP
#define EMP_COMPUTE_MANAGER_HPP
#include "vulkan/device.hpp"
namespace emp {
class ComputeManager {
public:
    ComputeManager(Device& device);
    ~ComputeManager();
    [[nodiscard]] VkCommandBuffer getCurrentCommandBuffer() const {
        assert(m_isComputeStarted &&
               "Cannot get command buffer when frame not in progress");
        return m_command_buffers[m_frame_index];
    }
    VkCommandBuffer beginCompute(Renderer& renderer);
    void endCompute();
private:
    void createCommandBuffer();
    void freeCommandBuffers();

    void createSyncObjects();
    void freeSyncObjects();

    std::vector<VkFence> m_compute_in_flight_fences;
    std::vector<VkSemaphore> m_compute_finished_semaphores;
    int m_frame_index = 0;
    bool m_isComputeStarted = false;
    std::vector<VkCommandBuffer> m_command_buffers;
    Device& m_device;

};
};
#endif //EMP_COMPUTE_MANAGER_HPP
