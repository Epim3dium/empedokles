#include "compute_manager.hpp"
namespace emp {
ComputeManager::ComputeManager(Device& device) : m_device(device) {
    createCommandBuffer();
}
ComputeManager::~ComputeManager() {
    freeCommandBuffers();
}
void ComputeManager::freeCommandBuffers() {
}
void ComputeManager::createCommandBuffer() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = m_device.getCommandPool();
    allocInfo.commandBufferCount = static_cast<uint32_t>(1U);

    if (vkAllocateCommandBuffers(
                m_device.device(), &allocInfo, &m_command_buffer
        ) != VK_SUCCESS) 
    {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}
};

