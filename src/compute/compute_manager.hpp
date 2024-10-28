#ifndef EMP_COMPUTE_MANAGER_HPP
#define EMP_COMPUTE_MANAGER_HPP
#include "vulkan/device.hpp"
namespace emp {
class ComputeManager {
public:
    ComputeManager(Device& device);
    ~ComputeManager();
    [[nodiscard]] VkCommandBuffer getCurrentCommandBuffer() const {
        assert(isComputeStarted &&
               "Cannot get command buffer when frame not in progress");
        return m_command_buffer;
    }
    VkCommandBuffer beginCompute();
    void endCompute();
private:
    void createCommandBuffer();
    void freeCommandBuffers();
    bool isComputeStarted = false;
    VkCommandBuffer m_command_buffer;
    Device& m_device;
};
};
#endif //EMP_COMPUTE_MANAGER_HPP
