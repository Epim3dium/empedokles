#ifndef EMP_RENDERER_HPP
#define EMP_RENDERER_HPP

#include "io/window.hpp"
#include "vulkan/device.hpp"
#include "vulkan/swap_chain.hpp"

// std
#include <cassert>
#include <functional>
#include <memory>
#include <vector>

namespace emp {
struct ComputeManager;
class Renderer {
public:
    Renderer(Window& window, Device& device);

    ~Renderer();

    Renderer(const Renderer&) = delete;

    Renderer& operator=(const Renderer&) = delete;

    [[nodiscard]] VkRenderPass getSwapChainRenderPass() const {
        return m_swapChain->getRenderPass();
    }

    [[nodiscard]] float getAspectRatio() const {
        return m_swapChain->extentAspectRatio();
    }

    [[nodiscard]] bool isFrameInProgress() const {
        return m_isFrameStarted;
    }

    [[nodiscard]] VkCommandBuffer getCurrentCommandBuffer() const {
        assert(m_isFrameStarted &&
               "Cannot get command buffer when frame not in progress");
        return m_command_buffers[m_current_frame_index];
    }

    [[nodiscard]] int getFrameIndex() const {
        assert(m_isFrameStarted &&
               "Cannot get frame index when frame not in progress");
        return m_current_frame_index;
    }

    VkCommandBuffer beginFrame();

    void endFrame();

    void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);

    void endSwapChainRenderPass(VkCommandBuffer commandBuffer) const;
private:
    void setComputeSemaphore(VkSemaphore semaphore) { 
        m_compute_finished_semaphore = semaphore;
    }

    void createCommandBuffers();

    void freeCommandBuffers();

    void recreateSwapChain();

    Window& m_window;
    Device& m_device;
    std::unique_ptr<SwapChain> m_swapChain;
    std::vector<VkCommandBuffer> m_command_buffers;
    VkSemaphore m_compute_finished_semaphore = NULL;

    uint32_t m_current_image_index{};
    int m_current_frame_index{0};
    bool m_isFrameStarted{false};
    friend ComputeManager;
};
} // namespace emp

#endif
