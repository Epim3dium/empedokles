#include "compute_manager.hpp"
#include "graphics/renderer.hpp"
#include "vulkan/swap_chain.hpp"
namespace emp {
ComputeManager::ComputeManager(Device& device) : m_device(device) {
    createCommandBuffer();
    createSyncObjects();
}
ComputeManager::~ComputeManager() {
    freeCommandBuffers();
    freeSyncObjects();
}

void ComputeManager::freeSyncObjects() {
    for (size_t i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(
                m_device.device(), m_compute_finished_semaphores[i], nullptr
        );
        vkDestroyFence(
                m_device.device(), m_compute_in_flight_fences[i], nullptr
        );
    }
}
void ComputeManager::freeCommandBuffers() {
    vkFreeCommandBuffers(
            m_device.device(),
            m_device.getComputeCommandPool(),
            static_cast<uint32_t>(3),
            m_command_buffers.data()
    );
    m_command_buffers.clear();
}
void ComputeManager::createSyncObjects() {
    m_compute_in_flight_fences.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
    m_compute_finished_semaphores.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(m_device.device(),
                &semaphoreInfo,
                nullptr,
                &m_compute_finished_semaphores[i]) != VK_SUCCESS ||
            vkCreateFence(
                m_device.device(), &fenceInfo, nullptr, &m_compute_in_flight_fences[i]) !=
                VK_SUCCESS) {
            throw std::runtime_error(
                "failed to create compute synchronization objects for a "
                "frame!");
        }
    }
}
VkCommandBuffer ComputeManager::beginCompute(Renderer& renderer) {
    m_frame_index = renderer.getFrameIndex();
    vkWaitForFences(m_device.device(), 1, &m_compute_in_flight_fences[m_frame_index], VK_TRUE, UINT64_MAX);
    vkResetFences(m_device.device(), 1, &m_compute_in_flight_fences[m_frame_index]);
    vkResetCommandBuffer(m_command_buffers[m_frame_index], /*VkCommandBufferResetFlagBits*/ 0);

    m_isComputeStarted = true;

    auto commandBuffer = getCurrentCommandBuffer();
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }
    renderer.setComputeSemaphore(m_compute_finished_semaphores[m_frame_index]);
    return commandBuffer;
}
void ComputeManager::endCompute() {
    assert(m_isComputeStarted &&
           "Can't call endFrame while frame is not in progress");
    auto commandBuffer = getCurrentCommandBuffer();
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
    // vkResetCommandBuffer(m_command_buffers[m_frame_index],
    // /*VkCommandBufferResetFlagBits*/ 0);
    // recordComputeCommandBuffer(m_command_buffers[m_frame_index]);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_command_buffers[m_frame_index];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &m_compute_finished_semaphores[m_frame_index];
    // in swapchain:
    //VkSemaphore waitSemaphores[] = { computeFinishedSemaphores[currentFrame], imageAvailableSemaphores[currentFrame] };

    if (vkQueueSubmit(m_device.computeQueue(),
            1,
            &submitInfo,
            m_compute_in_flight_fences[m_frame_index]) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit compute command buffer!");
    };

    m_isComputeStarted = false;
}
void ComputeManager::createCommandBuffer() {
    m_command_buffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = m_device.getComputeCommandPool();
    allocInfo.commandBufferCount = static_cast<uint32_t>(m_command_buffers.size());

    if (vkAllocateCommandBuffers(
                m_device.device(), &allocInfo, m_command_buffers.data()
        ) != VK_SUCCESS) 
    {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}
};

